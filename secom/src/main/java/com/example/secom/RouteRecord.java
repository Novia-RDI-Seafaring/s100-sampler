package com.example.secom;

import jakarta.persistence.*;
import java.time.LocalDateTime;

@Entity
@Table(name = "route_records")
public class RouteRecord {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false, unique = true)
    private String uuid;

    @Column(nullable = false)
    private String filename;

    @Column(nullable = false)
    private LocalDateTime uploadTime;

    private LocalDateTime downloadTime;

    // Getters and Setters
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

    public String getFilename() {
        return filename;
    }

    public void setFilename(String filename) {
        this.filename = filename;
    }

    public LocalDateTime getUploadTime() {
        return uploadTime;
    }

    public void setUploadTime(LocalDateTime uploadTime) {
        this.uploadTime = uploadTime;
    }

    public LocalDateTime getDownloadTime() {
        return downloadTime;
    }

    public void setDownloadTime(LocalDateTime downloadTime) {
        this.downloadTime = downloadTime;
    }

    @Lob
    @Column(name = "data", nullable = false)
    private byte[] data;

    public byte[] getData() {
        return data;
    }
    public void setData(byte[] data) {
        this.data = data;
    }
}
