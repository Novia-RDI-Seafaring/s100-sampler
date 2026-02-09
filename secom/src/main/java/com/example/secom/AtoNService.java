package com.example.secom;

import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

@Service
public class AtoNService {

    private final AtoNRecordRepository fileRecordRepository;

    public AtoNService(AtoNRecordRepository fileRecordRepository) {
        this.fileRecordRepository = fileRecordRepository;
    }

    public AtoNRecord saveFile(String uuid, String filename) {
        AtoNRecord record = new AtoNRecord();
        record.setUuid(uuid);
        record.setFilename(filename);
        record.setUploadTime(LocalDateTime.now());
        return fileRecordRepository.save(record);
    }

    public List<AtoNRecord> getAllFiles() {
        return fileRecordRepository.findAll(); // Retrieve all records
    }

    public AtoNRecord updateDownloadTime(Long id) {
        AtoNRecord record = fileRecordRepository.findById(id).orElseThrow(() ->
                new RuntimeException("FileRecord not found with ID: " + id));
        record.setDownloadTime(LocalDateTime.now());
        return fileRecordRepository.save(record); // Update record
    }

    public List<AtoNRecord> getUndownloadedFiles() {
        // Fetch records where downloadTime is null
        return fileRecordRepository.findByDownloadTimeIsNull();
    }
    public List<String> getAllFileUuids() {
        return fileRecordRepository.findAll()
                .stream()
                .map(AtoNRecord::getUuid)
                .collect(Collectors.toList());
    }

    public Optional<AtoNRecord> getFileByUuid(String uuid) {
        return fileRecordRepository.findByUuid(uuid);
    }
}
