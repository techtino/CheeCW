import sqlite3
example_string = "1,2,3,4,5,6,7,8,9"
numbas = list(map(int, example_string.split(',')))

con = sqlite3.connect('example.db')
cur = con.cursor()

cur.execute("CREATE TABLE Numbers (id integer primary key, number integer)")
for number in numbas:
    cur.execute("INSERT INTO Numbers (number) VALUES (?)", (number,))

cur.execute("SELECT number FROM Numbers")
rows = cur.fetchall()
for row in rows:
    print(row[0], end =' ')
con.commit()