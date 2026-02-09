package com.example.secom;

import org.springframework.data.jpa.repository.JpaRepository;
import java.util.List;
import java.util.Optional;

public interface AcknowledgementRepository extends JpaRepository<AcknowledgementRecord, String> {
    List<AcknowledgementRecord> findByAckType(int ackType);
    Optional<AcknowledgementRecord> findByTransactionIdentifier(String transactionIdentifier);
}
