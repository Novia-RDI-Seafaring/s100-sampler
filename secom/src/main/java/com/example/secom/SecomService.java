package com.example.secom;

import org.grad.secom.core.interfaces.AcknowledgementSecomInterface;
import org.grad.secom.core.interfaces.PingSecomInterface;
import org.grad.secom.core.interfaces.UploadSecomInterface;
import org.grad.secom.core.interfaces.CapabilitySecomInterface;
import org.grad.secom.core.interfaces.SubscriptionSecomInterface;
import org.grad.secom.core.models.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class SecomService implements UploadSecomInterface, CapabilitySecomInterface, AcknowledgementSecomInterface, PingSecomInterface, SubscriptionSecomInterface {

    private final SecomUpload uploadSecomInterface; // Final field for upload
    private final SecomCapability capabilitySecomInterface; // Final field for capability
    private final SecomAck ackSecomInterface;
    private final SecomSubscription subscrptionSecomInterface;

    @Autowired
    public SecomService(SecomUpload uploadSecomInterface, SecomCapability capabilitySecomInterface, SecomAck ackSecomInterface, SecomSubscription subscrptionSecomInterface) {
        this.uploadSecomInterface = uploadSecomInterface; // Constructor injection for upload
        this.capabilitySecomInterface = capabilitySecomInterface; // Constructor injection for capability
        this.ackSecomInterface = ackSecomInterface;
        this.subscrptionSecomInterface = subscrptionSecomInterface;
    }

    @Override
    public UploadResponseObject upload(UploadObject uploadObject) {
        return uploadSecomInterface.upload(uploadObject); // Delegate to the existing upload functionality
    }

    @Override
    public AcknowledgementResponseObject acknowledgment(AcknowledgementObject ackObject) {
        return ackSecomInterface.acknowledgment(ackObject); // Delegate to the existing upload functionality
    }

    @Override
    public CapabilityResponseObject capability() {
        return capabilitySecomInterface.capability(); // Delegate to the existing capability functionality
    }

    public SubscriptionNotificationResponseObject notification() {
        return subscrptionSecomInterface.notification(); // Delegate to the existing capability functionality
    }


    @Override
    public PingResponseObject ping() {
        return null;
    }

    @Override
    public SubscriptionResponseObject subscription(@jakarta.validation.Valid SubscriptionRequestObject subscriptionRequestObject) {
        return null;
    }
}
