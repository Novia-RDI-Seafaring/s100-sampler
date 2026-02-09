package com.example.secom;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;

public interface AtoNRecordRepository extends JpaRepository<AtoNRecord, Long> {
    Optional<AtoNRecord> findByUuid(String uuid);
    List<AtoNRecord> findByDownloadTimeIsNull();
}
