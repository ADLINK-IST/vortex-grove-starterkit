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

	/* Get the program parameters
	 * Parameters: rotaryPublisher [Number of cycles] */
	int cycles;

	if (argv[1])
	{
			cycles = atoi(argv[1]); 
			cout << "=== [rotaryPublisher] Cycles: " << cycles << endl;
	}
	else
	{
		cycles = 10000;
		cout << "=== [rotaryPublisher] Cycles: 10,000, CTRL - C to exit." << endl;
	}


    /** A dds::domain::DomainParticipant is created for the default domain. */
    dds::domain::DomainParticipant participant(org::opensplice::domain::default_id());
	cout << "=== [rotaryPublisher] Participant Created" << endl;
	
	/* A dds::topic::qos::TopicQos is created with Reliability set to Reliable to
	* guarantee delivery. */

    dds::topic::qos::TopicQos reliableTopicQos = participant.default_topic_qos() 
		<< dds::core::policy::Reliability::Reliable();

    /* Set the Reliable TopicQos as the new default */
    participant.default_topic_qos(reliableTopicQos);
	
	/* A dds::topic::Topic is created for the Analogue Sensor types on the
	* domain participant.
	* The Topic data type used for the sensors is defined in the IDL file.
	* Here we create a Topic of type VortexGrove::AnalogueSensor called
	* VortexGrove_RotarySensor */

	dds::topic::Topic<VortexGrove::AnalogueSensor> RotarySensorTopic(participant,
		"VortexGrove_RotarySensor", reliableTopicQos);
	cout << "=== [rotaryPublisher] Topic VortexGrove_RotarySensor Created " << endl;

	/* Create a Publisher and Data Writer.
	 * This Publisher will use the Analogue Sensor Topic to pass Temperature and 
	 * Lux values into the DDS */

	dds::pub::qos::PublisherQos pubQos = participant.default_publisher_qos();

	dds::pub::Publisher rotaryPublisher(participant, pubQos);

	dds::pub::qos::DataWriterQos dwQos = RotarySensorTopic.qos();

	dds::pub::DataWriter<VortexGrove::AnalogueSensor> sensorDW(rotaryPublisher, RotarySensorTopic, dwQos);
	cout << "=== [rotaryPublisher] Publisher and DataWriter Created " << endl;

	/* Initialise Grove Pi Board */

	if (init() == -1)
	{
		cout << "=== [rotaryPublisher] GrovePi Failed to initialise" << endl;
		cout << "=== [rotaryPublisher] Check the GrovePi is online (green LED on top)\n"
			 << "=== [rotaryPublisher] Test the I2C port connection with command: \"i2cdetect -y 1\"" << endl;
		return 0;
	}
	else
	{
		cout << "=== [rotaryPublisher] Grove Pi initialised" << endl;
	}

    /* Create a unique reference for the Potentiometer values.
	 * This reference is also used as the Topic Key */

	short Rotary = 100;

	/* For Rotary Angle Sensor
	* Reference voltage of ADC is 5v
	* Vcc of the grove interface is normally 5v
	* Full Rotary Angle is 300 DEG */
	int	adc_ref = 5;
	int	grove_vcc = 5;
	int full_angle = 300;
	float voltage;
	float degrees;


	int count = 0;
	int rotaryData = 0;

	int rotaryPin = 1;

	/* Set the pins to input */
	pinMode(1, 0);

	/* Create a Topic Instance of the grove_AnalogueSensor Topic */
	VortexGrove::AnalogueSensor	currVal;
	
	for (int i = 0; i < cycles; i++) 
	{

		/* Read data from pin 1 */
		rotaryData = analogRead(rotaryPin);
		
		/* Pass the data to the Topic Instance */
		currVal.id() = Rotary;
		currVal.rValue() = rotaryData;

		/* Calculate the Degrees based on the potentiometer value */
		voltage = round((float)(rotaryData)* adc_ref / 1023);
		degrees = round((voltage * full_angle) / grove_vcc);
		
		currVal.value() = degrees;
		
		cout << "=== [rotaryPublisher] Sending Sample No.[" << count << "]"
	 << "=== [rotaryPublisher] Data: " << rotaryData << endl;

		/* Send the data sample to the data writer */
		sensorDW << currVal; 
		
		/* Wait for a second */
		sleep(1);
		count++;
		
	}
    return 0;
}
