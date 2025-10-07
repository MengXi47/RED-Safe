"""Helpers for talking to the IPC scan gRPC service."""

import json
import logging
from typing import Any, Optional, Union

import grpc

from . import ipcscan_pb2, ipcscan_pb2_grpc

LOGGER = logging.getLogger(__name__)


def scan_ipc_dynamic(
    target: str = "localhost:20001",
    timeout: Optional[float] = 5.0,
) -> Union[dict[str, Any], str, None]:
    """
    Call IPCScanService.Scan and return the parsed payload when possible.

    Returns the decoded JSON object when the service replies with valid JSON,
    otherwise returns the raw string payload. None is returned if the RPC fails.
    """
    channel = grpc.insecure_channel(target)
    stub = ipcscan_pb2_grpc.IPCScanServiceStub(channel)
    request = ipcscan_pb2.ScanRequest()
    try:
        response = stub.Scan(request, timeout=timeout)
    except grpc.RpcError as exc:
        LOGGER.warning("IPC scan gRPC call failed: %s", exc)
        return None

    payload = response.result
    try:
        return json.loads(payload)
    except json.JSONDecodeError:
        LOGGER.debug("IPC scan response is not JSON. Returning raw payload.")
        return payload
