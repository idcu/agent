FROM debian:bookworm-slim AS builder

LABEL maintainer="IDCU Agent Team"
LABEL description="IDCU Agent Builder"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY . .

RUN mkdir -p build && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF .. \
    && ninja

FROM debian:bookworm-slim AS runtime

LABEL maintainer="IDCU Agent Team"
LABEL description="IDCU Agent Runtime"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/build/bin/idcu-agent /app/
COPY --from=builder /build/build/lib/ /app/lib/
COPY config/ /app/config/

ENV LD_LIBRARY_PATH=/app/lib:$LD_LIBRARY_PATH

EXPOSE 8080

CMD ["/app/idcu-agent"]
