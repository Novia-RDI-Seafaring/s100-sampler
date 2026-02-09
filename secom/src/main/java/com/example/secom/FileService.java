package com.example.secom;

import com.example.secom.FileRecord;
import com.example.secom.FileRecordRepository;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

@Service
public class FileService {

    private final FileRecordRepository fileRecordRepository;

    public FileService(FileRecordRepository fileRecordRepository) {
        this.fileRecordRepository = fileRecordRepository;
    }

    public FileRecord saveFile(String uuid, String filename) {
        FileRecord record = new FileRecord();
        record.setUuid(uuid);
        record.setFilename(filename);
        record.setUploadTime(LocalDateTime.now());
        return fileRecordRepository.save(record);
    }

    public List<FileRecord> getAllFiles() {
        return fileRecordRepository.findAll(); // Retrieve all records
    }

    public FileRecord updateDownloadTime(Long id) {
        FileRecord record = fileRecordRepository.findById(id).orElseThrow(() ->
                new RuntimeException("FileRecord not found with ID: " + id));
        record.setDownloadTime(LocalDateTime.now());
        return fileRecordRepository.save(record); // Update record
    }

    public List<FileRecord> getUndownloadedFiles() {
        // Fetch records where downloadTime is null
        return fileRecordRepository.findByDownloadTimeIsNull();
    }
    public List<String> getAllFileUuids() {
        return fileRecordRepository.findAll()
                .stream()
                .map(FileRecord::getUuid)
                .collect(Collectors.toList());
    }

    public Optional<FileRecord> getFileByUuid(String uuid) {
        return fileRecordRepository.findByUuid(uuid);
    }
}
