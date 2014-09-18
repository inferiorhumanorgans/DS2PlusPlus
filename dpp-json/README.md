This file describes DPP Json v1.0

##Common Elements##

Files adhering to the DPP-JSON v1.0 specification should contain one JSON object at the root containing the following properties:

###Root Elemenets###

####`dpp_version`####
An integer representing the version of DPP-JSON used to structure this file.  Currently 1.

####`file_version`####
An integer representing the revision of the file.  Increment by one every time you change the file.

####`file_mtime`####
A string representing the last time this file was modified.

####`file_type`####
A string representing the DPP-JSON subtype.  Either "ecu" or "string_table"

####`uuid`####
A string representing an RFC 4122 GUID.  Must be unique across all files and object types.  Required.

##ECU Definition File##

###Root Elements###

####`address`####
A string containing a byte in hexadecimal form (ex: 0x00) that represents the DS2 address of the ECUs supported by this file, or null if this is the root ECU.  Required.

####`family`####
A string representing BMW's common name (ex: DME, DDE, EWS, etc.) for this type of ECU.  Required.

####`name`####
A string representing the plain-text description of the ECU being described.  Required.

####`parent_id`####
A string representing the RFC 4122 GUID of the parent ECU, or null if this is the root ECU.  Required.

####`hardware_number`####
A string containing a byte in hexadecimal form (ex: 0x00) that represents the hardware version of the ECUs supported by this file.  Fuzzy match.  Only present in non-root files.  Required.

####`software_number`####
A string containing a byte in hexadecimal form (ex: 0x00) that represents the software version of the ECUs supported by this file.  Fuzzy match.  Only present in non-root files.  Required.

####`coding_index`####
A string containing a byte in hexadecimal form (ex: 0x00) that represents the coding index of the ECUs supported by this file.  Fuzzy match.  Only present in non-root files.  Required.

####`part_number`####
An integer representing the part number of the ECUs that this file describes.  Exact match.  Only present in non-root files.  Required.

####`endian`####
A string indicating the byte sex of an ECU.  Either big or little.  Required.

####`operations`####
A hash with the keys being strings representing the names of the operations supported by this ECU, and the values being valid operation objects.  This is merged recursively with all parent ECUs, with items higher up on the tree taking a lower precedence.  Required.

###Operation Object###

####`uuid`####
A string representing an RFC 4122 GUID.  Must be unique across all files and object types.  Required.

####`command`####
An array of strings with each element representing a byte in hexadecimal form.  This array represents the payload portion of the DS2 packet sent to the ECU for this operation.  Required.

####`results`####
A hash with the keys representing the name of the result element, and the values being valid result objects.

###Result Object####

####`uuid`####
A string representing an RFC 4122 GUID.  Must be unique across all files and object types.  Required.

####`type`####
A string representing the strategy used to decode this result.  Required.  One of: `6bit-string`, `boolean`, `byte`, `hex_string`, `short`, `string`.

####`display`####
A string representing the display format used to present this result.  Required.  One of: `enum`, `float`, `hex_int`, `hex_string`, `int`, `raw`, `string`.

####`start_pos`####
An integer representing the location of this result within the DS2 packet.  Offset relative to the start of the packet's payload.  Index 0 is the response status.  Required.

####`length`####
An integer representing the length of the data within the DS2 packet.  Must be suitable given the `type` property given.  `byte` and `boolean` *MUST* be 1.  `short` *MUST* be 2.  `string` and `hex_string` *MUST* be greater than 0.

####`mask`####
For the `boolean` type, a string representing a byte in hexadecimal form that represents the bitmask used to determine the boolean value.

####`levels`####
For `byte` types with `enum` display formats and `boolean` types, levels is a hash representing the various options.  For `boolean` types the keys *MUST* be `yes` and `no`.  For `enum` types, the values must be strings representing the various bitmaks.  Additionally `all` and `else` and `none` are supported by `enum` values.

####`factor_a`####
A floating point number representing the multiplicative factor used to resolve floating point values from the DS2 packet.  Required for `float` display types.

####`factor_b`####
A floating point number representing the additive factor used to resolve floating point values from the DS2 packet.  Optional for `float` display types.  Default is 0.

####`rpn`####
A space delimited string representing a series of Reverse Polish Notation commands to run.  Base 16 numbers are supported (starts with 0x) as are base 10.  The following commands are supported:

* `N` - push the current byte from the result
* `+` - pop two from the stack, add, push result
* `-` - pop two from the stack, subtract, push difference
* `*` - pop two from the stack, multiply, push product
* `/` - pop two from the stack, divide, push result
* `>>` - pop two from the stack, shift right, push result
* `<<` - pop two from the stack, shift left, push result

Currently RPN is only observed on byte types and all numbers are assumed to be integers.

This should probably replace `mask`, `factor_a`, and `factor_b`.
