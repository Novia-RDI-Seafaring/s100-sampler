package com.example.secom;

import org.grad.secom.core.models.*;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.web.bind.annotation.RestController;
import org.grad.secom.core.interfaces.CapabilitySecomInterface;
import org.grad.secom.core.models.enums.ContainerTypeEnum;
import org.grad.secom.core.models.enums.SECOM_DataProductType;

import java.util.Collections;

@SpringBootApplication
@RestController // Add this annotation to treat this class as a REST controller
public class SecomCapability implements CapabilitySecomInterface {

    /**
     * GET /v1/capability : Returns the service instance capabilities.
     *
     * @return the SECOM-compliant service capabilities
     */
    public CapabilityResponseObject capability() {
        // Populate the implemented SECOM interfaces
        ImplementedInterfaces implementedInterfaces = new ImplementedInterfaces();
        implementedInterfaces.setUpload(true);

        // Start building the capability entry
        CapabilityObject capabilityObject = new CapabilityObject();
        capabilityObject.setContainerType(ContainerTypeEnum.S100_DataSet);
        capabilityObject.setDataProductType(SECOM_DataProductType.S212);
        capabilityObject.setImplementedInterfaces(implementedInterfaces);

        // Start building the capability response
        CapabilityResponseObject capabilityResponseObject = new CapabilityResponseObject();
        capabilityResponseObject.setCapability(Collections.singletonList(capabilityObject));
        // capabilityObject.setServiceVersion(this.appVersion);

        // And return the Capability Response Object
        return capabilityResponseObject;
    }

}
