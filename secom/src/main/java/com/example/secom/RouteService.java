package com.example.secom;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;
import org.springframework.stereotype.Service;

@Service
public class RouteService {

    private final RouteRepository fileRecordRepository;

    public RouteService(RouteRepository fileRecordRepository) {
        this.fileRecordRepository = fileRecordRepository;
    }

    public RouteRecord saveFile(String uuid, String filename) {
        RouteRecord record = new RouteRecord();
        record.setUuid(uuid);
        record.setFilename(filename);
        record.setUploadTime(LocalDateTime.now());
        return fileRecordRepository.save(record);
    }
    public RouteRecord save(RouteRecord record) {
        return fileRecordRepository.save(record);
    }

    public List<RouteRecord> getAllFiles() {
        return fileRecordRepository.findAll(); // Retrieve all records
    }

    public RouteRecord updateDownloadTime(Long id) {
        RouteRecord record = fileRecordRepository.findById(id).orElseThrow(() ->
                new RuntimeException("Route Record not found with ID: " + id));
        record.setDownloadTime(LocalDateTime.now());
        return fileRecordRepository.save(record); // Update record
    }

    public List<RouteRecord> getUndownloadedFiles() {
        // Fetch records where downloadTime is null
        return fileRecordRepository.findByDownloadTimeIsNull();
    }
    public List<String> getAllFileUuids() {
        return fileRecordRepository.findAll()
                .stream()
                .map(RouteRecord::getUuid)
                .collect(Collectors.toList());
    }

    public Optional<RouteRecord> getFileByUuid(String uuid) {
        return fileRecordRepository.findByUuid(uuid);
    }


}
