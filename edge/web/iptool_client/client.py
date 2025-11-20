from __future__ import annotations

import logging
from dataclasses import dataclass
from typing import Sequence

import grpc

from . import iptool_pb2, iptool_pb2_grpc

LOGGER = logging.getLogger(__name__)


@dataclass(frozen=True)
class NetworkConfigResult:
    """簡化後的 IPtool 網路設定結果。"""

    interface_name: str
    ip_address: str
    subnet_mask: str
    gateway: str
    dns: list[str]
    mode: str
    mode_raw: str


@dataclass(frozen=True)
class OperationResult:
    """描述 IPtool 設定操作結果。"""

    success: bool
    message: str


def _split_dns(value: str) -> list[str]:
    """將 IPtool 以分號/逗號串接的 DNS 字串轉為陣列。"""

    if not value:
        return []
    separators = [",", ";", " "]
    entries: list[str] = [value]
    for sep in separators:
        entries = [piece for chunk in entries for piece in chunk.split(sep)]
    return [entry.strip() for entry in entries if entry.strip()]


def _mode_label(mode: int) -> str:
    """將 enum 轉為易讀字串。"""

    if mode == iptool_pb2.NetworkMode.NETWORK_MODE_DHCP:
        return "DHCP"
    if mode == iptool_pb2.NetworkMode.NETWORK_MODE_MANUAL:
        return "MANUAL"
    return "UNSPECIFIED"


class IPToolClient:
    """與 edge/IPtool gRPC 服務通訊的輕量封裝。"""

    def __init__(self, target: str, timeout_seconds: float = 3.0) -> None:
        self._target = target
        self._timeout = timeout_seconds

    def get_network_config(self, interface_name: str) -> NetworkConfigResult | None:
        """查詢指定介面的網路設定。失敗時回傳 None。"""

        if not interface_name:
            LOGGER.warning("get_network_config called without interface_name")
            return None
        request = iptool_pb2.GetNetworkConfigRequest(interface_name=interface_name)
        try:
            with grpc.insecure_channel(self._target) as channel:
                stub = iptool_pb2_grpc.NetworkServiceStub(channel)
                response = stub.GetNetworkConfig(request, timeout=self._timeout)
        except grpc.RpcError as exc:
            LOGGER.warning("IPtool gRPC call failed: %s", exc)
            return None

        config = response.config
        try:
            mode_raw = iptool_pb2.NetworkMode.Name(config.mode)
        except ValueError:
            mode_raw = f"UNRECOGNIZED_{config.mode}"
        return NetworkConfigResult(
            interface_name=config.interface_name or interface_name,
            ip_address=config.ip_address or "",
            subnet_mask=config.subnet_mask or "",
            gateway=config.gateway or "",
            dns=_split_dns(config.dns),
            mode=_mode_label(config.mode),
            mode_raw=mode_raw,
        )

    def update_network_config(
        self,
        *,
        interface_name: str,
        ip_address: str,
        subnet_mask: str,
        gateway: str,
        dns: list[str],
    ) -> OperationResult:
        """設定指定介面的手動 IP。"""

        clean_dns = ",".join(dns)
        config = iptool_pb2.NetworkConfig(
            interface_name=interface_name,
            ip_address=ip_address,
            subnet_mask=subnet_mask,
            gateway=gateway,
            dns=clean_dns,
            mode=iptool_pb2.NetworkMode.NETWORK_MODE_MANUAL,
        )
        request = iptool_pb2.UpdateNetworkConfigRequest(config=config)
        try:
            with grpc.insecure_channel(self._target) as channel:
                stub = iptool_pb2_grpc.NetworkServiceStub(channel)
                response = stub.UpdateNetworkConfig(request, timeout=self._timeout)
        except grpc.RpcError as exc:
            LOGGER.warning("IPtool update call failed: %s", exc)
            return OperationResult(False, f"IPtool 連線失敗：{exc}")
        message = response.message or ("成功更新介面 " + interface_name if response.success else "更新失敗")
        return OperationResult(bool(response.success), message)

    def switch_to_dhcp(self, interface_name: str) -> OperationResult:
        """將指定介面切換為 DHCP。"""

        request = iptool_pb2.SwitchToDhcpRequest(interface_name=interface_name)
        try:
            with grpc.insecure_channel(self._target) as channel:
                stub = iptool_pb2_grpc.NetworkServiceStub(channel)
                response = stub.SwitchToDhcp(request, timeout=self._timeout)
        except grpc.RpcError as exc:
            LOGGER.warning("IPtool SwitchToDhcp call failed: %s", exc)
            return OperationResult(False, f"IPtool 連線失敗：{exc}")
        message = response.message or ("已切換介面 " + interface_name if response.success else "切換失敗")
        return OperationResult(bool(response.success), message)


__all__: Sequence[str] = ["IPToolClient", "NetworkConfigResult", "OperationResult"]
