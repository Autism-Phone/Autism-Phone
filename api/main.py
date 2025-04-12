from fastapi import FastAPI
import mysql.connector
from pydantic import BaseModel

db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="h4s10",
    database="test"
)
cursor = db.cursor()

app = FastAPI()

class Item(BaseModel):
    number: int

@app.get("/items/")
def get_items():
    cursor.execute("SELECT * FROM test_table")
    items = cursor.fetchall()
    return {"items": [{"number": row[0]} for row in items]}

@app.post("/items/")
def create_item(item: Item):
    sql = "INSERT INTO test_table (number) VALUES (%s)"
    cursor.execute(sql, (item.number,))
    db.commit()
    return {"message": "Item added", "id": cursor.lastrowid}