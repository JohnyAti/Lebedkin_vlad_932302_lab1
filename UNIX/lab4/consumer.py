import json
import socket
import time
from kafka import KafkaConsumer

class TicketConsumer:
    def __init__(self):
        self.consumer = KafkaConsumer(
            'tickets',
            bootstrap_servers='kafka:9092',
            group_id='ticket-consumers',
            value_deserializer=lambda x: json.loads(x.decode('utf-8'))
        )
        self.consumer_id = socket.gethostname()
        self.stats = {'total': 0}
    
    def process(self, event):
        event_type = event.get('type', 'unknown')
        self.stats['total'] += 1
        
        if event_type == 'ticket_reserved':
            print(f"[{self.consumer_id}]  RESERVE: {event['user']} - ${event['price']}")
        elif event_type == 'ticket_purchased':
            print(f"[{self.consumer_id}]  BUY: {event['ticket_id']} - ${event.get('final_price', '?')}")
        elif event_type == 'ticket_cancelled':
            print(f"[{self.consumer_id}]  CANCEL: {event['ticket_id']} - {event['user']}")
        else:
            print(f"[{self.consumer_id}] 🔸 {event_type.upper()}: {event.get('user', '?')}")
    
    def show_stats(self):
        print(f"\n[{self.consumer_id}] Stats: total={self.stats['total']}")
        for key, value in list(self.stats.items())[:5]:
            if key != 'total' and value > 0:
                print(f"  {key}: {value}")
    
    def run(self):
        print(f"[{self.consumer_id}] Consumer started")
        last_stat = time.time()
        
        try:
            for message in self.consumer:
                self.process(message.value)
                
                if time.time() - last_stat > 30:
                    self.show_stats()
                    last_stat = time.time()
        
        except KeyboardInterrupt:
            self.show_stats()
            print(f"[{self.consumer_id}] Stopped")

if __name__ == "__main__":
    TicketConsumer().run()