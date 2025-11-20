"""WebRTC signaling server that streams the mosaic frames to browsers."""

from __future__ import annotations

import asyncio
import functools
import logging
import threading
from typing import Any, Dict, Set

from aiohttp import web
from aiortc import RTCPeerConnection, RTCSessionDescription, VideoStreamTrack
from av import VideoFrame

from .mosaic import MosaicComposer

LOGGER = logging.getLogger(__name__)


class MosaicVideoTrack(VideoStreamTrack):
    """Video track backed by the MosaicComposer output."""

    kind = "video"

    def __init__(self, composer: MosaicComposer) -> None:
        super().__init__()
        self._composer = composer
        self._last_timestamp: int | None = None

    async def recv(self) -> VideoFrame:
        loop = asyncio.get_running_loop()
        frame, ts = await loop.run_in_executor(
            None,
            functools.partial(self._composer.wait_for_frame, self._last_timestamp, 2.0),
        )
        self._last_timestamp = ts
        video_frame = VideoFrame.from_ndarray(frame, format="bgr24")
        video_frame.pts, video_frame.time_base = await self.next_timestamp()
        return video_frame


class WebRTCServer:
    """Minimal signaling server wrapping aiohttp for Answer generation."""

    def __init__(self, composer: MosaicComposer, host: str, port: int) -> None:
        self._composer = composer
        self._host = host
        self._port = port
        self._loop: asyncio.AbstractEventLoop | None = None
        self._thread: threading.Thread | None = None
        self._shutdown_event: asyncio.Event | None = None
        self._pcs: Set[RTCPeerConnection] = set()
        self._site: web.TCPSite | None = None
        self._ready = threading.Event()

    def start(self) -> None:
        if self._thread:
            return
        self._loop = asyncio.new_event_loop()
        self._thread = threading.Thread(target=self._run, args=(self._loop,), daemon=True)
        self._thread.start()
        self._ready.wait(timeout=5.0)

    def stop(self) -> None:
        if not self._loop:
            return
        if self._shutdown_event is None:
            self._ready.wait(timeout=5.0)
        if not self._shutdown_event:
            return
        self._loop.call_soon_threadsafe(self._shutdown_event.set)
        if self._thread:
            self._thread.join(timeout=5.0)
            self._thread = None
        self._loop = None

    def _run(self, loop: asyncio.AbstractEventLoop) -> None:
        asyncio.set_event_loop(loop)
        loop.run_until_complete(self._serve())

    async def _serve(self) -> None:
        self._shutdown_event = asyncio.Event()
        app = web.Application()
        app.router.add_route("POST", "/webrtc/offer", self._handle_offer)
        app.router.add_route("OPTIONS", "/webrtc/offer", self._handle_options)

        runner = web.AppRunner(app)
        await runner.setup()
        self._site = web.TCPSite(runner, self._host, self._port)
        await self._site.start()
        self._ready.set()
        LOGGER.info("WebRTC signaling server listening on %s:%s", self._host, self._port)

        await self._shutdown_event.wait()
        await self._close_all()
        await runner.cleanup()

    async def _handle_offer(self, request: web.Request) -> web.Response:
        payload: Dict[str, Any] = await request.json()
        offer = RTCSessionDescription(sdp=payload["sdp"], type=payload["type"])
        pc = RTCPeerConnection()
        self._pcs.add(pc)

        track = MosaicVideoTrack(self._composer)
        pc.addTrack(track)

        @pc.on("connectionstatechange")
        async def _on_state_change() -> None:
            LOGGER.info("Peer connection state: %s", pc.connectionState)
            if pc.connectionState in {"failed", "closed"}:
                await self._cleanup_pc(pc)

        await pc.setRemoteDescription(offer)
        answer = await pc.createAnswer()
        await pc.setLocalDescription(answer)
        LOGGER.info("Generated WebRTC answer for client.")
        headers = {
            "Access-Control-Allow-Origin": "*",
            "Access-Control-Allow-Headers": "Content-Type",
            "Access-Control-Allow-Methods": "POST, OPTIONS",
        }
        return web.json_response(
            {"sdp": pc.localDescription.sdp, "type": pc.localDescription.type},
            headers=headers,
        )

    async def _handle_options(self, _request: web.Request) -> web.Response:
        headers = {
            "Access-Control-Allow-Origin": "*",
            "Access-Control-Allow-Headers": "Content-Type",
            "Access-Control-Allow-Methods": "POST, OPTIONS",
        }
        return web.Response(status=204, headers=headers)

    async def _cleanup_pc(self, pc: RTCPeerConnection) -> None:
        if pc in self._pcs:
            self._pcs.discard(pc)
        for sender in pc.getSenders():
            try:
                sender.track and sender.track.stop()
            except Exception:
                LOGGER.debug("Failed to stop sender track cleanly.", exc_info=True)
        try:
            await pc.close()
        except Exception:
            LOGGER.exception("Failed to close peer connection cleanly.")

    async def _close_all(self) -> None:
        while self._pcs:
            pc = self._pcs.pop()
            await self._cleanup_pc(pc)
