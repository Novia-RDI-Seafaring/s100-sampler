package com.example.secom;
import org.springframework.data.jpa.repository.JpaRepository;
import java.util.List;
import java.util.Optional;

public interface RouteRepository extends JpaRepository<RouteRecord, Long> {
    Optional<RouteRecord> findByUuid(String uuid);
    List<RouteRecord> findByDownloadTimeIsNull();
}
