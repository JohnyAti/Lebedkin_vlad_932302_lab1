import os
import time
import json
import uuid
import random
from datetime import datetime
from kafka import KafkaProducer

class TicketProducer:
    def __init__(self):
        self.producer = KafkaProducer(
            bootstrap_servers='kafka:9092',
            value_serializer=lambda v: json.dumps(v).encode('utf-8')
        )
        self.users = [f"user{i}@email.com" for i in range(5)]
        self.tickets = {}
    
    def create_event(self):
        action = random.choice(['reserve', 'buy', 'cancel'])
        
        if action == 'reserve':
            ticket_id = f"TICKET-{uuid.uuid4().hex[:6]}"
            event = {
                'type': 'ticket_reserved',
                'ticket_id': ticket_id,
                'user': random.choice(self.users),
                'price': random.randint(50, 300),
                'time': datetime.now().isoformat()
            }
            self.tickets[ticket_id] = event
            return event
        
        elif action == 'buy' and self.tickets:
            tid = random.choice(list(self.tickets.keys()))
            event = {
                'type': 'ticket_purchased',
                'ticket_id': tid,
                'user': self.tickets[tid]['user'],
                'final_price': self.tickets[tid]['price'],
                'time': datetime.now().isoformat()
            }
            return event
        
        elif action == 'cancel' and self.tickets:
            tid = random.choice(list(self.tickets.keys()))
            event = {
                'type': 'ticket_cancelled',
                'ticket_id': tid,
                'user': self.tickets[tid]['user'],
                'time': datetime.now().isoformat()
            }
            del self.tickets[tid]
            return event
        
        return {
            'type': action,
            'user': random.choice(self.users),
            'time': datetime.now().isoformat()
        }
    
    def run(self):
        print("Producer: sending events to Kafka...")
        count = 0
        
        try:
            while True:
                event = self.create_event()
                self.producer.send('tickets', event)
                print(f"Sent: {event['type']} (ID: {event.get('ticket_id', 'N/A')})")
                count += 1
                
                if count % 10 == 0:
                    print(f"Total events sent: {count}")
                
                time.sleep(2)
                
        except KeyboardInterrupt:
            print("\nProducer stopped")
        finally:
            self.producer.close()

if __name__ == "__main__":
    TicketProducer().run()