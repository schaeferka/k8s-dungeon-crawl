FROM python:3.9-slim

WORKDIR /app

RUN apt-get update && apt-get install -y \
    build-essential \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt

RUN useradd --create-home --home-dir /home/appuser appuser
USER appuser

COPY . .

EXPOSE 5000

ENV FLASK_ENV=production

CMD ["flask", "run", "--host=0.0.0.0", "--port=5000"]
