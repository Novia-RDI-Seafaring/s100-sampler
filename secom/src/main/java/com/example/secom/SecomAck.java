package com.example.secom;

import org.grad.secom.core.interfaces.AcknowledgementSecomInterface;
import org.grad.secom.core.models.AcknowledgementObject;
import org.grad.secom.core.models.AcknowledgementResponseObject;
import org.grad.secom.core.models.EnvelopeAckObject;
import org.grad.secom.core.models.enums.AckTypeEnum;
import org.grad.secom.core.models.enums.NackTypeEnum;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.stereotype.Component;
import org.springframework.validation.annotation.Validated;
import javax.validation.Valid;
import jakarta.ws.rs.Path;

import java.util.Objects;

/**
 * Implementation of the SECOM Acknowledgement Interface.
 */
@SpringBootApplication
@Component
@Path("/") // Root path of SECOM interfaces
@Validated
public class SecomAck implements AcknowledgementSecomInterface {

    private final Logger logger = LoggerFactory.getLogger(SecomAck.class);

    @Autowired
    private AcknowledgementService acknowledgementService;


    @Override
    public AcknowledgementResponseObject acknowledgment(@Valid AcknowledgementObject acknowledgementObject) {
        AcknowledgementResponseObject response = new AcknowledgementResponseObject();
        try {
            logger.info("Received acknowledgement request!");

            // Extract envelope
            EnvelopeAckObject envelope = acknowledgementObject.getEnvelope();

            // Validate SECOM fields
            if (envelope.getAckType() == null) {
                logger.error("Missing AckType in acknowledgement object");
                response.setMessage("Missing AckType");
                return response;
            }
            if (envelope.getEnvelopeCertificate() == null || envelope.getEnvelopeCertificate().isEmpty()) {
                logger.error("Missing Envelope Certificate in acknowledgement object");
                response.setMessage("Missing Envelope Certificate");
                return response;
            }
            if (envelope.getTransactionIdentifier() == null || Objects.equals(envelope.getTransactionIdentifier().toString(), "")) {
                logger.error("Missing Transaction Identifier in acknowledgement object");
                response.setMessage("Missing Transaction Identifier");
                return response;
            }

            if (envelope.getNackType() != null) {
                logger.warn("Negative Acknowledgement received: " + envelope.getNackType().name());
                response.setMessage("Negative Acknowledgement: " + envelope.getNackType().name());
            } else {
                response.setMessage("Acknowledgement successfully processed");
            }

            acknowledgementService.saveAcknowledgement(acknowledgementObject);

            response.setMessage("Acknowledgement saved successfully");

            // Log successful processing
            logger.info("Acknowledgement successfully processed for transaction: " + envelope.getTransactionIdentifier());
        } catch (Exception e) {
            logger.error("Error processing acknowledgement", e);
            response.setMessage("Error processing acknowledgement: " + e.getMessage());
        }
        return response;
    }

}
