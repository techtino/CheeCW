import sqlite3

def saveToDatabase():
    example_string = "1,2,3,4,5,6,7,8,9"
    numberList = list(map(int, example_string.split(',')))

    con = sqlite3.connect('example.db')
    cur = con.cursor()

    cur.execute("CREATE TABLE Numbers (id integer primary key, number integer)")
    for number in numberList:
        cur.execute("INSERT INTO Numbers (number) VALUES (?)", (number,))

    cur.execute("SELECT number FROM Numbers")
    rows = cur.fetchall()
    for row in rows:
        print(row[0], end =' ')
    con.commit()
saveToDatabase()