FROM python:3.9-slim

WORKDIR /app

RUN apt-get update && apt-get install -y curl \
    build-essential \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# Install kubectl
RUN curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl" \
    && chmod +x kubectl \
    && mv kubectl /usr/local/bin/
    
COPY requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt

RUN useradd --create-home --home-dir /home/appuser appuser
USER appuser

COPY . .
COPY app/static/images/armor.png /app/static/images/
COPY app/static/images/ring.png /app/static/images/
COPY app/static/images/sword.png /app/static/images/

EXPOSE 5000

ENV FLASK_ENV=production

CMD ["flask", "run", "--host=0.0.0.0", "--port=5000"]
