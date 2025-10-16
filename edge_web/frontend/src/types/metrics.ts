export interface MetricsResponse {
  cpu: { percent: number };
  ram: { used_mb: number; total_mb: number; percent: number };
  disk: { used_gb: number; total_gb: number; percent: number };
  temperature: string;
}
