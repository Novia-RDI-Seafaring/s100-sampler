package com.example.secom;

import jakarta.persistence.*;
import java.time.LocalDateTime;

@Entity
@Table(name = "acknowledgements")
public class AcknowledgementRecord {

    @Id
    private String transactionIdentifier; // UUID as primary key

    @Column(nullable = false)
    private String envelopeCertificate;

    @Column
    private String envelopeRootCertificateThumbprint;

    @Column(nullable = false)
    private int ackType;

    @Column
    private Integer nackType; // Nullable

    @Column(nullable = false)
    private LocalDateTime createdAt;

    // Getters and Setters
    public String getTransactionIdentifier() {
        return transactionIdentifier;
    }

    public void setTransactionIdentifier(String transactionIdentifier) {
        this.transactionIdentifier = transactionIdentifier;
    }

    public String getEnvelopeCertificate() {
        return envelopeCertificate;
    }

    public void setEnvelopeCertificate(String envelopeCertificate) {
        this.envelopeCertificate = envelopeCertificate;
    }

    public String getEnvelopeRootCertificateThumbprint() {
        return envelopeRootCertificateThumbprint;
    }

    public void setEnvelopeRootCertificateThumbprint(String envelopeRootCertificateThumbprint) {
        this.envelopeRootCertificateThumbprint = envelopeRootCertificateThumbprint;
    }

    public int getAckType() {
        return ackType;
    }

    public void setAckType(int ackType) {
        this.ackType = ackType;
    }

    public Integer getNackType() {
        return nackType;
    }

    public void setNackType(Integer nackType) {
        this.nackType = nackType;
    }

    public LocalDateTime getCreatedAt() {
        return createdAt;
    }

    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }
}
