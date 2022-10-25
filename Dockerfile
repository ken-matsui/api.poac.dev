FROM rust:1.63 as builder

WORKDIR /builder
COPY . .
RUN cargo build --release

FROM rust:1.63

WORKDIR /app
COPY --from=builder /builder/target/release/poac_api .
COPY /etc/secrets/root.crt /root/.postgresql/root.crt
EXPOSE 8080
CMD ["/app/poac_api"]
