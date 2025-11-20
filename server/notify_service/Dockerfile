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
# 預設圖片路徑（供程式讀取），也可在部署時覆蓋
ENV LOGO_PATH=/app/assets/RED_Safe_icon1.png

RUN apt-get update \
    && apt-get install -y --no-install-recommends curl \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m appuser
WORKDIR /app

# 放入 JAR 與圖片
COPY --from=build --chown=appuser:appuser /workspace/target/app.jar /app/app.jar
COPY --chown=appuser:appuser RED_Safe_icon1.png /app/assets/RED_Safe_icon1.png

# 確保權限
RUN chown -R appuser:appuser /app && chmod 644 /app/assets/RED_Safe_icon1.png

USER appuser
EXPOSE 30041 30042

HEALTHCHECK --interval=30s --timeout=5s --start-period=30s --retries=3 \
    CMD curl -f http://localhost:30041/health || exit 1

ENTRYPOINT ["java","-XX:MaxRAMPercentage=75","-Duser.timezone=Asia/Taipei","-Dserver.port=30041","-Dgrpc.server.port=30042","-jar","/app/app.jar"]
