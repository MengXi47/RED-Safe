# syntax=docker/dockerfile:1

FROM node:20-slim AS frontend-builder

WORKDIR /app/frontend

COPY frontend/package.json frontend/package-lock.json ./
RUN npm ci

COPY frontend/ .
RUN npm run build

FROM python:3.11-slim

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    DJANGO_SETTINGS_MODULE=raspberry_contral.settings \
    DJANGO_STATIC_ROOT=/app/staticfiles

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    libpq-dev \
    libffi-dev \
    libssl-dev \
    libopus-dev \
    libvpx-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt .

RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

COPY . .
COPY --from=frontend-builder /app/static/frontend ./static/frontend

RUN python manage.py collectstatic --noinput

EXPOSE 80

CMD ["gunicorn", "--bind", "0.0.0.0:80", "raspberry_contral.wsgi:application"]
