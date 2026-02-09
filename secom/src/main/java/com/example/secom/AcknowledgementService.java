package com.example.secom;

import org.grad.secom.core.models.EnvelopeAckObject;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import java.time.LocalDateTime;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

import org.grad.secom.core.models.AcknowledgementObject;

@Service
public class AcknowledgementService {

    @Autowired
    private AcknowledgementRepository repository;

    public void saveAcknowledgement(AcknowledgementObject acknowledgementObject) {
        EnvelopeAckObject envelope = acknowledgementObject.getEnvelope();

        AcknowledgementRecord record = new AcknowledgementRecord();
        record.setTransactionIdentifier(String.valueOf(envelope.getTransactionIdentifier()));
        record.setEnvelopeCertificate(envelope.getEnvelopeCertificate());
        record.setAckType(envelope.getAckType().getValue());
        record.setNackType(envelope.getNackType() != null ? envelope.getNackType().ordinal() : null);
        record.setCreatedAt(LocalDateTime.now());

        repository.save(record);
    }
    public Optional<Integer> getAckTypeByTransactionId(String transactionIdentifier) {
        return repository.findByTransactionIdentifier(transactionIdentifier)
                .map(AcknowledgementRecord::getAckType);
    }
    public Map<String, Integer> getAllAckTypes() {
        // Fetch all transaction identifiers and their corresponding types from your data source
        return repository.findAll().stream()
                .collect(Collectors.toMap(AcknowledgementRecord::getTransactionIdentifier, AcknowledgementRecord::getAckType));
    }




}
