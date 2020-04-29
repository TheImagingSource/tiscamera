#####
State
#####

To allow for an easy
The format of this state is JSON.

A single property looks like this:

.. code-block:: json
                
                {
                    "Exposure": 3000
                }

A Property consists out of the fields 'name' and 'value'.

- name is a string containing the property identifier.
- value contains the actual value. The interpretation of this field
  is done automatically.

In the case of an error a message will the written to TCAM_LOG.
The property will be ignored.
                

.. code-block:: json

   {
       "serial": "19951234",
       "version": "0.0.1",
       "properties": {
                "Exposure": 3000,
                "Exposure Auto": false
       }
   }

The fields "serial" and "version" are optional to allow simple state descriptions for gst-launch.

Example:

.. code-block:: sh

   gst-launch-1.0 tcamsrc serial=12341234 state='{\"Exposure\":3000,"Exposure\ Auto\":false}' ! ....

Please keep in mind that shell environments have special requirements concerning special characters
such as ``"``, ``'`` and ``:space:`` .
   
When loading states from files in full grown applications it is recommended to include these values
as security measurements.
If the serial miss matches the loading will be aborted.
If the json version miss matches the loading will be aborted.
