# utils/logger.py
"""
全域日誌紀錄工具
"""
import logging
import os

def setup_logger(log_file: str):
    os.makedirs(os.path.dirname(log_file), exist_ok=True)
    logging.basicConfig(filename=log_file, level=logging.INFO,
                        format='%(asctime)s %(levelname)s:%(message)s')
    return logging.getLogger()

logger = setup_logger("D:/pytorch/output/logs.txt")
