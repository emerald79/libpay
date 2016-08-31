# LibPAY - The Toolkit for Smart Payment Applications

  LibPAY provides a toolkit meant to support the implementation of Linux(R)
based payment solutions.  It is currently focused on support for the EMV(R)
Contactless specifications, but might be extended to support contact based
solutions as well as non EMV(R) payment systems in the future.


## Disclaimer

  LibPAY is work in progress and it is still in its early stages of development.
All APIs have to be considered unstable and will be changed without prior
notice.

  While LibPAY attempts to comply to the EMV(R) Contactless specifications, it
has not been certified in any way.  Thus, LibPAY does not claim to be compliant
with any payment system specifications whatsoever.

  To the best of my knowledge LibPAY does not violate the EMVCo Terms of Use.
However, I am not a lawyer.  Should it turn out that LibPAY actually does
violate rights of EMVCo or any other third party, this will void all usage
rights on LibPAY.


## LibPAY Configuration

### LibPAY Configuration -- Data Dictionary

Name                                  | Description                                           | Format   | Template               | Tag        | Length
------------------------------------- | ----------------------------------------------------- | -------- | ---------------------- | ---------- | ------
LibPAY Configuration                  | Configuration of Entry Point and Level 2 Kernels      | complex  | --                     | 'FF81E371' | var.
Set of Combinations                   | Set of Combinations of AID and Kernel ID              | complex  | 'FF81E371'             | 'FF82E371' | var.
Transaction Types                     | Transaction Types the Set of Combinations apply to    | complex  | 'FF82E371'             | 'FF83E371' | var.
Transaction Type                      | Type of Transaction (ISO 8583:1987)                   | n 2      | 'FF83E371', 'FF8FE371' | '9C'       | 1
Combination                           | A Combination of an Application ID and a Kernel ID    | complex  | 'FF82E371'             | 'FF84E371' | var.
AID -- Terminal                       | Identifies the Application (ISO-IEC 7816-5)           | b        | 'FF84E371'             | '9F06'     | 5 - 16
Kernel ID                             | Uniquely identifies a Kernel installed in the Reader  | b        | 'FF84E371'             | 'DF86E371' | 3 - 8
Status Check Support                  | Flag: Reader is able to perform a Status Check        | b        | 'FF82E371'             | 'DF87E371' | 1
Zero Amount Allowed                   | Flag: Transaction with a zero amount is permitted     | b        | 'FF82E371'             | 'DF88E371' | 1
Reader Contactless Transaction Limit  | Limit for contactless transactions                    | n 12     | 'FF82E371'             | 'DF8AE371' | 6
Reader Contactless Floor Limit        | Floor limit for contactless transactions              | n 12     | 'FF82E371'             | 'DF8BE371' | 6
Terminal Floor Limit                  | Floor limit for transactions                          | n 12     | 'FF82E371'             | 'DF8CE371' | 6
Reader CVM Required Limit             | Limit above which Cardholder Verification is required | n 12     | 'FF82E371'             | 'DF8DE371' | 6
Terminal Transaction Qualifiers (TTQ) | Reader capabilities, requirements and preferences     | b 32     | 'FF82E371'             | '9F66'     | 4
Autorun                               | If present, enables and configures Autorun mode       | complex  | 'FF81E371'             | 'FF8FE371' | var.
Amount, Authorised                    | Authorised Amount of the Transaction                  | n 12     | 'FF8FF371'             | '9F02'     | 6
Terminal Settings                     | Information about Merchant, Acquirer and Terminal     | complex  | 'FF81E371'             | 'FF92E371' | var.
Acquirer Identifier                   | Uniquely identifies acquirer within payment system    | n 6 - 11 | 'FF92E371'             | '9F01'     | 6
Merchant Category Code                | Type of Business (ISO 8583:1987 Business Code)        | n 4      | 'FF92E371'             | '9F15'     | 2
Merchant Identifier                   | Uniquely identifiers Merchant if Acquirer is given    | ans 15   | 'FF92E371'             | '9F16'     | 15
Terminal Country Code                 | Country of Location of Terminal (ISO 3166)            | n 3      | 'FF92E371'             | '9F1A'     | 2
Terminal Identification               | Unique Location of Terminal at Merchant               | an 8     | 'FF92E371'             | '9F1C'     | 8
Terminal Type                         | Environment, Capabilities and Operational Control     | n 2      | 'FF92E371'             | '9F35'     | 1
Point-Of-Service (POS) Entry Mode     | First two digits of ISO 8353:1987 POS Entry Mode      | n 2      | 'FF92E371'             | '9F39'     | 1
Additional Terminal Capabilities      | Data Input and Output Capabilities of the Terminal    | b        | 'FF92E371'             | '9F40'     | 5
Merchant Name and Location            | Indicates the Name and Location of the Merchant       | ans      | 'FF92E371'             | '9F4E'     | var.

### LibPAY Configuration -- Structure

Tag        | Name                 | --                  | --                                   | --               | Mandatory or Optional
---------- | -------------------- | ------------------- | ------------------------------------ | ---------------- | ---------------------
'FF81E371' | LibPAY Configuration | --                  | --                                   | --               | M
--         | 'FF82E371'           | Set of Combinations | --                                   | --               | M
--         | --                   | 'FF83E371'          | Transaction Types                    | --               | M
--         | --                   |                     | '9C'                                 | Transaction Type | M
--         | --                   |                     | '9C'                                 | Transaction Type | O
--         | --                   | 'FF84E371'          | Combination                          | --               | M
--         | --                   | --                  | '9F06'                               | AID -- Terminal  | M
--         | --                   | --                  | 'DF86E371'                           | Kernel ID        | M
--         | --                   | 'FF84E371'          | Combination                          | --               | O
--         | --                   | --                  | '9F06'                               | AID              | M
--         | --                   | --                  | 'DF86E371'                           | Kernel ID        | M
--         | --                   | 'DF87E371'          | Status Check Support                 | --               | O
--         | --                   | 'DF88E371'          | Zero Amount Allowed                  | --               | O
--         | --                   | 'DF89E371'          | Extended Selection Support           | --               | O
--         | --                   | 'DF8AE371'          | Reader Contactless Transaction Limit | --               | O
--         | --                   | 'DF8BE371'          | Reader Contactless Floor Limit       | --               | O
--         | --                   | 'DF8CE371'          | Terminal Floor Limit                 | --               | O
--         | --                   | 'DF8DE371'          | Reader CVM Required Limit            | --               | O
--         | --                   | '9F66'              | Terminal Transaction Qualifiers      | --               | O
--         | 'FF82E371'           | Set of Combinations | --                                   | --               | O
--         | --                   | 'FF83E371'          | Transaction Types                    | --               | M
--         | --                   |                     | '9C'                                 | Transaction Type | M
--         | --                   | 'FF84E371'          | Combination                          | --               | M
--         | --                   | --                  | 'DF85E371'                           | AID              | M
--         | --                   | --                  | 'DF86E371'                           | Kernel ID        | M
--         | --                   | 'DF87E371'          | Status Check Supported               | --               | O
--         | --                   | 'DF88E371'          | Zero Amount Allowed                  | --               | O
--         | --                   | 'DF89E371'          | Extended Selection Supported         | --               | O
--         | --                   | 'DF8AE371'          | Reader Contactless Transaction Limit | --               | O
--         | --                   | 'DF8BE371'          | Reader Contactless Floor Limit       | --               | O
--         | --                   | 'DF8CE371'          | Terminal Floor Limit                 | --               | O
--         | --                   | 'DF8DE371'          | Reader CVM Required Limit            | --               | O
--         | --                   | '9F66'              | Terminal Transaction Qualifiers      | --               | O
--         | 'FF8FE371'           | Autorun             | --                                   | --               | O
--         | --                   | 'DF90E371'          | Transaction Type                     | --               | M
--         | --                   | '9F02'              | Amount, Authorised                   | --               | M
--         | 'FF92E371'           | Terminal Settings   | --                                   | --               | M
--         | --                   | '9F01'              | Acquirer Identifier                  | --               | M
--         | --                   | '9F15'              | Merchant Category Code               | --               | M
--         | --                   | '9F16'              | Merchant Identifier                  | --               | M
--         | --                   | '9F1A'              | Terminal Country Code                | --               | M
--         | --                   | '9F1C'              | Terminal Identification              | --               | M
--         | --                   | '9F35'              | Terminal Type                        | --               | M
--         | --                   | '9F39'              | Point-Of-Service (POS) Entry Mode    | --               | M
--         | --                   | '9F40'              | Additional Terminal Capabilities     | --               | M
--         | --                   | '9F4E'              | Merchant Name and Location           | --               | M
