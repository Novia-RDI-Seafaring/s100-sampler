package com.example.secom;

import org.springframework.data.jpa.repository.JpaRepository;
import com.example.secom.FileRecord;

import java.util.List;
import java.util.Optional;

public interface FileRecordRepository extends JpaRepository<FileRecord, Long> {
    Optional<FileRecord> findByUuid(String uuid);
    List<FileRecord> findByDownloadTimeIsNull();
}
