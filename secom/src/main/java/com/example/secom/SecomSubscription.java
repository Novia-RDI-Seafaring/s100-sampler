package com.example.secom;

import org.grad.secom.core.models.*;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.web.bind.annotation.RestController;
import org.grad.secom.core.interfaces.SubscriptionSecomInterface;

@SpringBootApplication
@RestController // Add this annotation to treat this class as a REST controller
public class SecomSubscription implements SubscriptionSecomInterface {
    @Override
    public SubscriptionResponseObject subscription(@jakarta.validation.Valid SubscriptionRequestObject subscriptionRequestObject) {
        return null;
    }
    public SubscriptionNotificationResponseObject notification(){
        SubscriptionNotificationResponseObject notificationObject = new SubscriptionNotificationResponseObject();

        notificationObject.setResponseText("Ok");
        return notificationObject;
    }
}