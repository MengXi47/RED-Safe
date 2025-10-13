"""Helpers for talking to the IPC scan gRPC service."""

import json
import logging
from typing import Any, Optional, Union

import grpc

from . import ipcscan_pb2, ipcscan_pb2_grpc

LOGGER = logging.getLogger(__name__)

FALLBACK_SCAN_PAYLOAD = [
    {"ip": "192.168.0.100", "mac": "24:5E:BE:08:A9:03", "name": "IPC"},
    {"ip": "192.168.0.110", "mac": "24:5E:BE:08:A9:03", "name": "IPC"},
    {"ip": "192.168.0.120", "mac": "24:5E:BE:08:A9:03", "name": "IPC"},
    {"ip": "192.168.0.130", "mac": "24:5E:BE:08:A9:03", "name": "IPC"},
    {"ip": "192.168.0.140", "mac": "90:E2:FC:10:A6:45", "name": "General"},
]


def scan_ipc_dynamic(
    target: str = "localhost:20001",
    timeout: Optional[float] = 5.0,
) -> Union[list[dict[str, Any]], dict[str, Any], str, None]:
    """
    Call IPCScanService.Scan and return the parsed payload when possible.

    Returns the decoded JSON object when the service replies with valid JSON,
    otherwise returns the raw string payload. When the gRPC call fails, a
    fallback payload is returned to keep downstream flows functional.
    """
    channel = grpc.insecure_channel(target)
    stub = ipcscan_pb2_grpc.IPCScanServiceStub(channel)
    request = ipcscan_pb2.ScanRequest()
    try:
        response = stub.Scan(request, timeout=timeout)
    except grpc.RpcError as exc:
        LOGGER.warning("IPC scan gRPC call failed: %s. Returning fallback data.", exc)
        return [item.copy() for item in FALLBACK_SCAN_PAYLOAD]

    payload = response.result
    try:
        return json.loads(payload)
    except json.JSONDecodeError:
        LOGGER.debug("IPC scan response is not JSON. Returning raw payload.")
        return payload
