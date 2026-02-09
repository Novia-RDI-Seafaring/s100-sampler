package com.example.secom;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.grad.secom.core.interfaces.UploadSecomInterface;
import org.grad.secom.core.models.EnvelopeUploadObject;
import org.grad.secom.core.models.UploadObject;
import org.grad.secom.core.models.UploadResponseObject;
import org.grad.secom.core.models.enums.ContainerTypeEnum;
import org.grad.secom.core.models.enums.DigitalSignatureAlgorithmEnum;
import org.grad.secom.core.models.enums.SECOM_DataProductType;
import org.grad.secom.core.models.enums.SECOM_ResponseCodeEnum;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.validation.annotation.Validated;
import org.springframework.beans.factory.annotation.Autowired;
import javax.validation.Valid;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.Base64;
import java.nio.charset.StandardCharsets;
import java.time.LocalDateTime;
import java.nio.file.Paths;
//import java.nio.file.Path;
import jakarta.ws.rs.Path;
import org.springframework.web.server.ResponseStatusException;

/**
 * Implementation of the SECOM Upload Interface.
 * This implementation directly pushes new messages to the frontend via WebSockets.
 */
@SpringBootApplication
@Component
@Path("/") // <-- This should be the root path of the SECOM interfaces
@Validated
public class SecomUpload implements UploadSecomInterface {

    private final Logger logger = LoggerFactory.getLogger(SecomUpload.class);


    @Autowired
    private FileService fileService; // Inject FileService

    @Autowired
    private AtoNService atonService;

    @Autowired
    private RouteService routeService;


    @Override
    public UploadResponseObject upload(@Valid UploadObject uploadObject) {
        UploadResponseObject response = new UploadResponseObject();

        try {
            logger.info("Received upload object!");
            // Validate SECOM fields
            EnvelopeUploadObject envelope = uploadObject.getEnvelope();

            if (envelope.getContainerType() != ContainerTypeEnum.NONE){
                logger.error("Invalid Container Type in the upload object");
            }
            SECOM_DataProductType dataProductType = envelope.getDataProductType();
            if (dataProductType == SECOM_DataProductType.S212){
                logger.info("Received S212 data");
                handleS212(envelope);
            } else if (dataProductType == SECOM_DataProductType.S125 || dataProductType == SECOM_DataProductType.S201 ){
                logger.info("Received S125/S201 data");
                handleS125(envelope);
            }   else if (dataProductType == SECOM_DataProductType.S421){
                logger.info("Received S421 data");
                handleS421(envelope);
            }

            else {
                logger.info("Unsupported data received");
                response.setResponseText("Unsupported data received, only S125, S212 and S421 supported");
                return response;
            }
            if (envelope.getExchangeMetadata().getDataProtection().booleanValue() != Boolean.FALSE){
                logger.error("Invalid DataProtection in the upload object");
            }
            if (!envelope.getExchangeMetadata().getProtectionScheme().equals("SECOM")){
                logger.error("Invalid protectionScheme in the upload object");
            }
            if (envelope.getExchangeMetadata().getCompressionFlag().booleanValue() != Boolean.FALSE)
            {
                logger.error("Invalid Compression flag in the upload object");
                //response.setSECOM_ResponseCode(SECOM_ResponseCodeEnum.MISSING_REQUIRED_DATA_FOR_SERVICE); // Error code for invalid fields
                response.setResponseText("Invalid SECOM fields");
                return response;
            }
            response.setResponseText("Upload processed successfully");

        } catch (Exception e) {
            logger.error("Error processing upload", e);
            //response.setSECOM_ResponseCode(SECOM_ResponseCodeEnum.SCHEMA_VALIDATION_ERROR); // Error code for processing error
            response.setResponseText(e.getMessage());
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "Error processing upload", e);
        }

        return response;
    }

    private void handleS212(EnvelopeUploadObject S212Envelope) throws IOException {
        //envelope.getExchangeMetadata().getDigitalSignatureValue();
        String transactionIdFileName = S212Envelope.getTransactionIdentifier().toString() + ".xml";

        String gml = new String(S212Envelope.getData(), StandardCharsets.UTF_8);
        byte[] decodedData = Base64.getDecoder().decode(gml);
        String decodedXml = new String(decodedData, StandardCharsets.UTF_8);
        // Save the XML file locally
        saveXmlToFile(decodedXml, transactionIdFileName);

        // Save metadata in the database
        FileRecord fileRecord = fileService.saveFile(S212Envelope.getTransactionIdentifier().toString(),transactionIdFileName);
        logger.info("File metadata saved with ID: " + fileRecord.getId());
    }

    private void handleS125(EnvelopeUploadObject S125Envelope) throws IOException {
        //envelope.getExchangeMetadata().getDigitalSignatureValue();
        String transactionIdFileName = S125Envelope.getTransactionIdentifier().toString() + ".xml";

        String gml = new String(S125Envelope.getData(), StandardCharsets.UTF_8);
        byte[] decodedData = Base64.getDecoder().decode(gml);
        String decodedXml = new String(decodedData, StandardCharsets.UTF_8);
        // Save the XML file locally
        saveAtonToFile(decodedXml, transactionIdFileName);

        // Save metadata in the database
        AtoNRecord fileRecord = atonService.saveFile(S125Envelope.getTransactionIdentifier().toString(),transactionIdFileName);
        logger.info("File metadata saved with ID: " + fileRecord.getId());
    }


    private void handleS421(EnvelopeUploadObject env) {
        // env.getData() should be base64 (string or bytes containing base64 text)
        final byte[] xmlBytes;

        // If env.getData() is String:
        // xmlBytes = Base64.getDecoder().decode(env.getData());

        // If env.getData() is byte[] containing ASCII base64:
        String b64 = new String(env.getData(), StandardCharsets.US_ASCII);
        xmlBytes = Base64.getDecoder().decode(b64);

        RouteRecord record = new RouteRecord();
        record.setUuid(env.getTransactionIdentifier().toString());
        record.setUploadTime(LocalDateTime.now());
        record.setData(xmlBytes);

        // DB-only: filename no longer needed; set something harmless if column is NOT NULL
        record.setFilename(record.getUuid() + ".xml");

        RouteRecord saved = routeService.save(record);
        logger.info("Route saved with ID: {}", saved.getId());
    }




    private void saveRouteToFile(String gml, String fileName) throws IOException {
        java.nio.file.Path directory = Paths.get("route_uploads"); // Directory to save files
        if (!Files.exists(directory)) {
            Files.createDirectories(directory); // Create directory if it doesn't exist
        }
        java.nio.file.Path filePath = directory.resolve(fileName);
        Files.write(filePath, gml.getBytes(StandardCharsets.UTF_8));
        logger.info("Route saved to " + filePath.toString());
    }

    private void saveAtonToFile(String gml, String fileName) throws IOException {
        java.nio.file.Path directory = Paths.get("aton_uploads"); // Directory to save files
        if (!Files.exists(directory)) {
            Files.createDirectories(directory); // Create directory if it doesn't exist
        }
        java.nio.file.Path filePath = directory.resolve(fileName);
        Files.write(filePath, gml.getBytes(StandardCharsets.UTF_8));
        logger.info("Aton saved to " + filePath.toString());
    }

    private void saveXmlToFile(String gml, String fileName) throws IOException {
        java.nio.file.Path directory = Paths.get("uploads"); // Directory to save files
        if (!Files.exists(directory)) {
            Files.createDirectories(directory); // Create directory if it doesn't exist
        }
        java.nio.file.Path filePath = directory.resolve(fileName);
        Files.write(filePath, gml.getBytes(StandardCharsets.UTF_8));
        logger.info("File saved to " + filePath.toString());
    }
}
