#!/bin/bash

# Command line parsing
INPUT="$1"
OUTPUT=CERN_UserCertificate

# Check if input is .p12
if [ ${INPUT: -4} != ".p12" ] ; then
    echo "CERN user certificate in p12 format required"
    echo "Firefox: Preferences > Advanced > View Certificates > Backup"
    exit 1
fi

# Create directory
mkdir -p ~/private

# Transform certificate
openssl pkcs12 -clcerts -nokeys -in ${INPUT} -out ~/private/${OUTPUT}.pem
openssl pkcs12 -nocerts -in ${INPUT} -out ~/private/${OUTPUT}.tmp.key
openssl rsa -in ~/private/${OUTPUT}.tmp.key -out ~/private/${OUTPUT}.key

# Remove temporary file and set rights
rm ~/private/${OUTPUT}.tmp.key
chmod 644 ~/private/${OUTPUT}.pem
chmod 400 ~/private/${OUTPUT}.key
