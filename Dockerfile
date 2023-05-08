FROM rust:1 as builder

WORKDIR /builder
COPY . .
RUN cargo build --release

FROM rust:1

# Install the CockroachDB SSL Cert
RUN curl --create-dirs -o $HOME/.postgresql/root.crt 'https://cockroachlabs.cloud/clusters/91f60fff-e9ea-4705-939e-de8c45f0c82e/cert'

WORKDIR /app
COPY --from=builder /builder/target/release/poac_api .

ENV PORT 8080
EXPOSE $PORT
CMD ["/app/poac_api"]
