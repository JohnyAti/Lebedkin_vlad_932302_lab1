#!/bin/bash
echo "Waiting for Kafka"
sleep 20

docker-compose exec kafka kafka-topics --create \
  --topic tickets \
  --partitions 3 \
  --replication-factor 1 \
  --if-not-exists \
  --bootstrap-server localhost:9092

echo "Kafka  created successfully!"