FROM python:3.11-slim

WORKDIR /omnios

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

RUN python -m pytest src/tests/test_engine.py -v || true

EXPOSE 8080

CMD ["python", "src/main_improved.py"]
