FROM rust:1.63 as builder

WORKDIR /builder
COPY . .
RUN cargo build --release

FROM rust:1.63

ARG ROOT_CRT
RUN mkdir /root/.postgresql
RUN echo "$ROOT_CRT" > /root/.postgresql/root.crt

WORKDIR /app
COPY --from=builder /builder/target/release/poac_api .
EXPOSE 8080
CMD ["/app/poac_api"]
