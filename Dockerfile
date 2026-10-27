# Multi-stage build for ultra-lightweight C binary
FROM alpine:latest AS builder
RUN apk add --no-cache gcc musl-dev make
WORKDIR /app
COPY . .
RUN make all

# Final minimal production runtime container (~12MB)
FROM alpine:latest
RUN apk add --no-cache musl
WORKDIR /app
COPY --from=builder /app/contact_app /app/contact_app
COPY --from=builder /app/contacts.csv /app/contacts.csv
COPY --from=builder /app/frontend /app/frontend

EXPOSE 8080
CMD ["./contact_app", "--port", "8080"]
