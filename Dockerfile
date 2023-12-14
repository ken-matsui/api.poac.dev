FROM rust:1 as builder

WORKDIR /builder
COPY . .
RUN cargo build --release

FROM rust:1

WORKDIR /app
COPY --from=builder /builder/target/release/poac_api .

ENV HOST "0.0.0.0"
ENV PORT 8080
EXPOSE $PORT
CMD ["/app/poac_api"]
