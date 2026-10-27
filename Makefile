CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -D_POSIX_C_SOURCE=200809L
TARGET = contact_app
TEST_TARGET = test_contact

SRC = main.c contact.c storage.c server.c cli.c
TEST_SRC = test_contact.c contact.c storage.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)

run-cli: $(TARGET)
	./$(TARGET) --cli

clean:
	rm -f $(TARGET) $(TEST_TARGET) test_contacts.csv

.PHONY: all test run run-cli clean
