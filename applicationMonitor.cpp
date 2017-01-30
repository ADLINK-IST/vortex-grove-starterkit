/*
 *                         Vortex OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2017 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>

#include "VortexGrove_DCPS.hpp"

 /* C API for GrovePi Module */
#include "grovepi.h"


using namespace std;

int main (int argc, char *argv[])
{

    /** A dds::domain::DomainParticipant is created for the default domain. */
    dds::domain::DomainParticipant participant(org::opensplice::domain::default_id());
	cout << "=== [applicationMonitor] Participant Created" << endl;
    
	/* A dds::topic::qos::TopicQos is created with Reliability set to Reliable to
     * guarantee delivery. */

    dds::topic::qos::TopicQos reliableTopicQos = participant.default_topic_qos() 
		<< dds::core::policy::Reliability::Reliable();

    /* Set the Reliable TopicQos as the new default */
    participant.default_topic_qos(reliableTopicQos);

	/* A dds::topic::Topic is created for the Rotary Angle Sensor, Light Sensor 
	 * and LED's  on the domain participant. 
	 * Both sensors are of type VortexGrove::AnalogueSensor and named
	 * VortexGrove_LightSensor and VortexGrove_RotarySensor
	 * The LED topic is of type VortexGrove::LED 
	 * and named VortexGrove_LED */

	dds::topic::Topic<VortexGrove::AnalogueSensor> LightSensorTopic(participant,	
		"VortexGrove_LightSensor", reliableTopicQos);
	cout << "=== [applicationMonitor] VortexGrove_LightSensor Topic Created " << endl;

	dds::topic::Topic<VortexGrove::AnalogueSensor> RotarySensorTopic(participant,
		"VortexGrove_RotarySensor", reliableTopicQos);
	cout << "=== [applicationMonitor] VortexGrove_RotarySensor Topic Created " << endl;

	dds::topic::Topic<VortexGrove::LED> LEDTopic(participant, "VortexGrove_LED", reliableTopicQos);
	cout << "=== [applicationMonitor] VortexGrove_LED Topic Created " << endl;

    /* Create a Subscriber with two Data Readers, one for each sensor Topic.
     * QoS Settings comply with that on the topic. 
	 * All default QoS values for an Entity can be found in the language reference guide. */

	dds::sub::qos::SubscriberQos subQos = participant.default_subscriber_qos();
	
	dds::sub::Subscriber applicationMonitor(participant, subQos);
	cout << "=== [applicationMonitor] Subscriber Created" << endl;

	dds::sub::qos::DataReaderQos drQos = LightSensorTopic.qos();

	dds::sub::DataReader<VortexGrove::AnalogueSensor> LightSensorDR(applicationMonitor, LightSensorTopic, drQos);
	cout << "=== [applicationMonitor] VortexGrove_LightSensor DataReader Created " << endl;

	dds::sub::DataReader<VortexGrove::AnalogueSensor> RotarySensorDR(applicationMonitor, RotarySensorTopic, drQos);
	cout << "=== [applicationMonitor] VortexGrove_RotarySensor Created " << endl;

	/* Create a Publisher and Data Writer.
	 * This Publisher will use the LED Topic to turn the light on or off */

	dds::pub::qos::PublisherQos pubQos = participant.default_publisher_qos();

	dds::pub::Publisher LEDPublisher(participant, pubQos);
	cout << "=== [applicationMonitor] Publisher Created " << endl;

	dds::pub::qos::DataWriterQos dwQos = LEDTopic.qos();

	dds::pub::DataWriter<VortexGrove::LED> LEDDW(LEDPublisher, LEDTopic, dwQos);
	cout << "=== [applicationMonitor] VortexGrove_LED DataWriter Created " << endl;

	/* Create a unique reference for the Temperature and Lux values.
	* This reference is also used as the Topic Key */
	short RotaryID = 100;
	short LuxID = 200;
	int count = 0;

	/* For Rotary Angle Sensor 
	 * Reference voltage of ADC is 5v 
	 * Vcc of the grove interface is normally 5v 
	 * Full Rotary Angle is 300 DEG */
	int	adc_ref = 5;
	int	grove_vcc = 5;
	int full_angle = 300;

	/* For Rotary Angle Sensor */
	float voltage;
	float degrees;
	float sensor_value;
	
	/* For Light Sensor */
	int LuxValue;

	/* These are the threshold values to trigger the LED's */
	int maxDegree = 200;
	int maxLux = 700;


	/* Create a Topic Instance of the VortexGrove_LED Topic */
	VortexGrove::LED LEDstate;



	cout << "=== [applicationMonitor] Waiting for samples..." << endl;
	while (1) 
	{
		/* Readers created we need to Loan some memory to store received samples
		* the memory is returned once the Loan runs out of scope 
		* The function take() removes samples automatically stored in the Data Reader's buffer */

		dds::sub::LoanedSamples<VortexGrove::AnalogueSensor> RotarySensorSamples = RotarySensorDR.take();

		for (dds::sub::LoanedSamples<VortexGrove::AnalogueSensor>::const_iterator rotarySample = RotarySensorSamples.begin();
			rotarySample < RotarySensorSamples.end(); rotarySample++)
		{ 
			/* Check the sample we received is valid */
			if (rotarySample->info().valid())
			{
					/* Get the raw Value from the sample */
					sensor_value = rotarySample->data().rValue();

					/* Use this value to calculate the Voltage on the potentiometer*/
					voltage = round((float)(sensor_value)* adc_ref / 1023);

					/* Using the voltage we can roughly calculate the angle of the potentiometer */
					degrees = round((voltage * full_angle) / grove_vcc);

					cout << "=== [applicationMonitor] Calculated Angle: [" << degrees << "]" << endl;

				/* If the received value is out of range change the LED state */
				if (degrees > maxDegree)
				{
					if (rotarySample->data().id() == RotaryID)
						cout << "=== [applicationMonitor] Angle exceeded." << endl;

					/* Add the an ID to the LED Topic, this can be anything
					 * but we are going set it to a number we can use it in the LEDSubscriber 
					 * to find out which sensor has created the error condition */
					LEDstate.id() = rotarySample->data().id();
					LEDstate.on() = true;

					/* Write the instance with completed fields to the DataWriter */
					LEDDW << LEDstate;
				}
			}
		} 

		dds::sub::LoanedSamples<VortexGrove::AnalogueSensor> LightSensorSamples = LightSensorDR.take();

		for (dds::sub::LoanedSamples<VortexGrove::AnalogueSensor>::const_iterator lightSample = LightSensorSamples.begin();
			lightSample < LightSensorSamples.end(); lightSample++)
		{
			/* Check the sample we received is valid */
			if (lightSample->info().valid())
			{
				/* Get the raw Value from the sample */
				LuxValue = lightSample->data().rValue();

				/* There are some calculations which can be made here to calculate the exact Luminosity
				 * over the photo-resistor but these tend to be inaccurate and are beyond the scope 
				 * of this example */

				cout << "=== [applicationMonitor] Lux: [" << LuxValue << "]" << endl;

				/* If the received value is out of range change the LED state */
				if (LuxValue > maxLux)
				{
					cout << "=== [applicationMonitor] Lux exceeded." << endl;

					/* Add the an ID to the LED Topic, this can be anything
					 * but we are going set it to a number we can recognise in the LEDSubscriber
					 * to find out which sensor has created the error condition */
					LEDstate.id() = lightSample->data().id();
					LEDstate.on() = true;

					/* Write the instance with completed fields to the DataWriter */
					LEDDW << LEDstate;
				}
			}
		}
		/* Vortex DDS communicates several times faster than the RaspberryPi / GrovePi 
		 * are capable of communicating, too many successive read or write calls between 
		 * the two cause IO errors. Sleep for half a second */
		sleep(1);

		/* Reset the error condition;
		 * In order to update freeboard which is listening to this topic we need 
		 * to send an LED off sample. */
		LEDstate.id() = LuxID;
		LEDstate.on() = false;
		LEDDW << LEDstate;
		LEDstate.id() = RotaryID;
		LEDstate.on() = false;
		LEDDW << LEDstate;

		count++;
	}

    return 0;
}
