package com.example.secom;

import org.grad.secom.core.models.*;
import org.grad.secom.core.models.enums.AckRequestEnum;
import org.grad.secom.core.models.enums.DigitalSignatureAlgorithmEnum;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.server.ResponseStatusException;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.ArrayList;
import java.util.UUID;
import java.util.Base64;

@RestController
public class SecomController {

    @Autowired
    private SecomService secomService; // Autowire the service class

    //private final SecomSignatureProviderImpl secomSignatureProvider;

    @Autowired
    private RouteService routeService;

    @Autowired
    private FileService fileService;

    @Autowired
    private AtoNService atonService;

    @Autowired
    private AcknowledgementService acknowledgementService;

    public SecomController( ) {
        //this.secomSignatureProvider = secomSignatureProvider;
    }

    /**
     * POST /v1/subscription/notification : Returns the service instance capabilities.
     */
    @PostMapping("/v1/subscription/notification")
    public ResponseEntity<java.lang.Object> notification() {
        Map<String, Object> response = new HashMap<>();
        response.put("message", secomService.notification().getResponseText());

        return ResponseEntity.ok(response); // ? Returns HTTP 200
    }


    /**
     * GET /v1/capability : Returns the service instance capabilities.
     */
    @GetMapping("/v1/capability")

    public CapabilityResponseObject capability() {
        try {
            String testMessage = "Verify Certificate";
            byte[] messageBytes = testMessage.getBytes(StandardCharsets.UTF_8);

            // Use a predefined algorithm (adjust as needed)
            DigitalSignatureAlgorithmEnum algorithm = DigitalSignatureAlgorithmEnum.ECDSA;

            // If validation passes, proceed
            return secomService.capability();
        } catch (Exception e) {
            throw new RuntimeException("Certificate verification failed", e);
        }
    }

    /**
     * GET /v1/ping : Returns the ping
     */
    @GetMapping("/v1/ping")

    public PingResponseObject ping() {
        PingResponseObject pingresponse = secomService.ping();
       return pingresponse;
    }

    /**
     * POST /v1/object : Uploads an object to SECOM service.
     */
    @PostMapping("/v1/object")
    public ResponseEntity<Map<String, Object>> upload(@RequestBody UploadObject uploadObject) {
        try {
            UploadResponseObject response = secomService.upload(uploadObject);

            Map<String, Object> transformedResponse = new HashMap<>();
            transformedResponse.put("message", response.getResponseText());

            if (response.getSECOM_ResponseCode() != null) {
                transformedResponse.put("SECOM_ResponseCode", response.getSECOM_ResponseCode());
            }

            return ResponseEntity.ok(transformedResponse); // Return HTTP 200 OK on success

        } catch (ResponseStatusException ex) {
            // Convert exception into HTTP error response
            Map<String, Object> errorResponse = new HashMap<>();
            errorResponse.put("error", ex.getReason());
            return ResponseEntity.status(ex.getStatusCode()).body(errorResponse);
        }
    }

    /**
     * GET /v1/object : Get object(s)
     */
    @GetMapping("/v1/object")
    public ResponseEntity<GetResponseObject> getObject(
            @RequestParam(name = "dataReference", required = false) String dataReference) {

        List<RouteRecord> records = routeService.getAllFiles();
        List<DataResponseObject> dataResponseObjects = new ArrayList<>();

        for (RouteRecord record : records) {
            if (dataReference == null || dataReference.equals(record.getUuid())) {
                SECOM_ExchangeMetadataObject tempObject = new  SECOM_ExchangeMetadataObject();
                ;
                DataResponseObject dro = new DataResponseObject();
                dro.setData(Base64.getEncoder().encode(record.getData()));              // Base64 string

                dro.setExchangeMetadata(tempObject);
                dro.setAckRequest(AckRequestEnum.DELIVERED_AND_OPENED_ACK_REQUESTED);

                dataResponseObjects.add(dro);
            }
        }

        if (dataReference != null && dataResponseObjects.isEmpty()) {
            throw new ResponseStatusException(
                    HttpStatus.NOT_FOUND,
                    "Object not found: " + dataReference
            );
        }

        PaginationObject pagination = new PaginationObject();
        pagination.setTotalItems(dataResponseObjects.size());
        pagination.setMaxItemsPerPage(dataResponseObjects.size());

        GetResponseObject response = new GetResponseObject();
        response.setDataResponseObject(dataResponseObjects);
        response.setPagination(pagination);

        return ResponseEntity.ok(response);
    }



    /**
     * GET /v1/object/summary : Get summary of objects
     */
    @GetMapping("/v1/object/summary")
    public ResponseEntity<GetSummaryResponseObject> summary() {
        try {
            List<RouteRecord> records = routeService.getAllFiles();

            List<SummaryObject> summaryObjects = new ArrayList<>();
            for (RouteRecord record : records) {
                SummaryObject so = new SummaryObject();
                so.setDataReference(UUID.fromString(record.getUuid()));
                summaryObjects.add(so);
            }

            PaginationObject pagination = new PaginationObject();
            pagination.setTotalItems(records.size());
            pagination.setMaxItemsPerPage(records.size());

            GetSummaryResponseObject response = new GetSummaryResponseObject();
            response.setSummaryObject(summaryObjects);
            response.setPagination(pagination);

            return ResponseEntity.ok(response);

        } catch (ResponseStatusException ex) {
            throw ex; // let Spring handle it properly
        }
    }



    /**
     * POST /v1/acknowledgement : Acknowledgement interface
     */
    @PostMapping("/v1/acknowledgement")
    public Map<String, Object> acknowledgement(@RequestBody AcknowledgementObject ackObject) {
        AcknowledgementResponseObject response = secomService.acknowledgment(ackObject); // Get the response from the service

        // Create the transformed response as a map
        Map<String, Object> transformedResponse = new HashMap<>();
        transformedResponse.put("message", response.getResponseText());

        // Only include SECOM_ResponseCode if it's not null
        if (response.getSECOM_ResponseCode() != null) {
            transformedResponse.put("SECOM_ResponseCode", response.getSECOM_ResponseCode());
        }

        return transformedResponse; // Return the transformed response
    }

    @GetMapping("/v1/ack/getType/{transactionIdentifier}")
    public ResponseEntity<Integer> getAcknowledgementType(@PathVariable("transactionIdentifier") String transactionIdentifier) {
        Optional<Integer> ackType = acknowledgementService.getAckTypeByTransactionId(transactionIdentifier);
        return ackType.map(ResponseEntity::ok).orElseGet(() -> ResponseEntity.notFound().build());
    }

    @GetMapping("/v1/ack/getAllTypes")
    public ResponseEntity<Map<String, Integer>> getAllAcknowledgementTypes() {
        Map<String, Integer> ackTypes = acknowledgementService.getAllAckTypes();
        return ResponseEntity.ok(ackTypes);
    }


    @GetMapping("/v1/files/undownloaded")
    public ResponseEntity<List<String>> getUndownloadedFilesContents() {
        List<FileRecord> undownloadedFiles = fileService.getUndownloadedFiles();

        // Read file contents
        List<String> fileContents = undownloadedFiles.stream()
                .peek(fileRecord -> fileService.updateDownloadTime(fileRecord.getId())) // Update download time
                .map(fileRecord -> new File("uploads/", fileRecord.getFilename()))
                .filter(File::exists) // Ensure file exists
                .map(file -> {
                    try {
                        return Files.readString(file.toPath()); // Read file content as string
                    } catch (IOException e) {
                        throw new RuntimeException("Error reading file: " + file.getAbsolutePath(), e);
                    }
                })
                .collect(Collectors.toList());

        return ResponseEntity.ok(fileContents);
    }
    
    @GetMapping("/v1/files/all")
    public ResponseEntity<List<String>> getAllFilesContents() {
        List<FileRecord> allFiles = fileService.getAllFiles();
        List<String> fileContents = allFiles.stream()
                .peek(fileRecord -> fileService.updateDownloadTime(fileRecord.getId())) // Update download time
                .map(fileRecord -> new File("uploads/", fileRecord.getFilename()))
                .filter(File::exists) // Ensure file exists
                .map(file -> {
                    try {
                        return Files.readString(file.toPath());
                    } catch (IOException e) {
                        throw new RuntimeException("Error reading file: " + file.getAbsolutePath(), e);
                    }
                })
                .collect(Collectors.toList());
        return ResponseEntity.ok(fileContents);
    }

    @GetMapping("/v1/atons")
    public ResponseEntity<List<String>> getAllAtonUuids() {
        List<String> uuids = atonService.getAllFileUuids();
        return uuids.isEmpty() ? ResponseEntity.noContent().build() : ResponseEntity.ok(uuids);
    }

    @GetMapping("/v1/atons/{uuid}")
    public ResponseEntity<String> downloadAtonByUuid(@PathVariable("uuid") String uuid) {
        Optional<AtoNRecord> atonRecordOpt = atonService.getFileByUuid(uuid);

        if (atonRecordOpt.isEmpty()) {
            return ResponseEntity.notFound().build();
        }

        File file = new File("aton_uploads/", atonRecordOpt.get().getFilename());
        if (!file.exists()) {
            return ResponseEntity.notFound().build();
        }

        try {
            atonService.updateDownloadTime(atonRecordOpt.get().getId());
            String content = Files.readString(file.toPath());
            return ResponseEntity.ok(content);
        } catch (IOException e) {
            throw new RuntimeException("Error reading file: " + file.getAbsolutePath(), e);
        }
    }


    @GetMapping("/v1/files")
    public ResponseEntity<List<String>> getAllFileUuids() {
        List<String> uuids = fileService.getAllFileUuids();
        return uuids.isEmpty() ? ResponseEntity.noContent().build() : ResponseEntity.ok(uuids);
    }

    @GetMapping("/v1/files/{uuid}")
    public ResponseEntity<String> downloadFileByUuid(@PathVariable("uuid") String uuid) {
        Optional<FileRecord> fileRecordOpt = fileService.getFileByUuid(uuid);

        if (fileRecordOpt.isEmpty()) {
            return ResponseEntity.notFound().build();
        }

        File file = new File("uploads/", fileRecordOpt.get().getFilename());
        if (!file.exists()) {
            return ResponseEntity.notFound().build();
        }

        try {
            fileService.updateDownloadTime(fileRecordOpt.get().getId());
            String content = Files.readString(file.toPath());
            return ResponseEntity.ok(content);
        } catch (IOException e) {
            throw new RuntimeException("Error reading file: " + file.getAbsolutePath(), e);
        }
    }



}
