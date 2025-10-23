FROM eclipse-temurin:17-jdk-jammy AS build
ENV TZ=Asia/Taipei
WORKDIR /workspace

COPY mvnw .
COPY .mvn .mvn
COPY pom.xml .
COPY src src

RUN chmod +x mvnw \
    && ./mvnw -B clean package -DskipTests \
    && JAR_PATH=$(find target -maxdepth 1 -type f -name "*.jar" ! -name "*original*" | head -n 1) \
    && [ -n "$JAR_PATH" ] \
    && mv "$JAR_PATH" target/app.jar

FROM eclipse-temurin:17-jre-jammy
ENV TZ=Asia/Taipei

RUN apt-get update \
    && apt-get install -y --no-install-recommends curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /workspace/target/app.jar /app/app.jar

USER root

EXPOSE 30011 30012

HEALTHCHECK --interval=30s --timeout=5s --start-period=30s --retries=3 \
    CMD curl -f http://localhost:30011/health || exit 1

ENTRYPOINT ["java","-XX:MaxRAMPercentage=75","-Duser.timezone=Asia/Taipei","-Dserver.port=30011","-Dgrpc.server.port=30012","-jar","/app/app.jar"]
