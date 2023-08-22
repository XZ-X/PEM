#!/bin/bash

mkdir -p CVE-cases

cat cmp-ret/eval-coreutils/coreutils.gcc94O0.vs.gcc94O3.pkl.dumped.csv|egrep "^single_binary_main_chown," > CVE-cases/case-18018.pem.csv
cat cmp-ret/eval-how-help/find47.locate.O0vsO3.pkl.dumped.csv|egrep "^visit_old_format," > CVE-cases/case-2452.pem.csv
cat cmp-ret/eval-trex/libmagick7.so.O0vsO3.pkl.dumped.csv|egrep "^sRGBTransformImage," > CVE-cases/case-20311.pem.csv
cat cmp-ret/eval-trex/libmagick7.so.O0vsO3.pkl.dumped.csv|egrep "^WaveImage," > CVE-cases/case-20309.pem.csv
cat cmp-ret/eval-trex/libmagick7.so.O0vsO3.pkl.dumped.csv|egrep "^ComplexImages," > CVE-cases/case-13308.pem.csv
cat cmp-ret/eval-trex/libz.so.O0vsO3.pkl.dumped.csv|egrep "^inflateMark," > CVE-cases/case-9842.pem.csv
cat cmp-ret/eval-trex/openssl101f.O0vsO3.pkl.dumped.csv|egrep "^X509_verify_cert," > CVE-cases/case-4044.pem.csv
cat cmp-ret/eval-trex/openssl101f.O0vsO3.pkl.dumped.csv|egrep "^EVP_PKEY_decrypt," > CVE-cases/case-3711.pem.csv



cat ml-baseline-results/trex/similarity_eval2_coreutils_O0O3.csv.formatted.csv|egrep "^single_binary_main_chown-coreutils," > CVE-cases/case-18018.trex.csv
cat ml-baseline-results/trex/similarity_eval3_findutils470.locate_O0O3.csv.formatted.csv|egrep "^visit_old_format-" > CVE-cases/case-2452.trex.csv
cat ml-baseline-results/trex/similarity_eval2_libmagick7_O0O3.csv.formatted.csv|egrep "^sRGBTransformImage-" > CVE-cases/case-20311.trex.csv
cat ml-baseline-results/trex/similarity_eval2_libmagick7_O0O3.csv.formatted.csv|egrep "^WaveImage-" > CVE-cases/case-20309.trex.csv
cat ml-baseline-results/trex/similarity_eval2_libmagick7_O0O3.csv.formatted.csv|egrep "^ComplexImages-" > CVE-cases/case-13308.trex.csv
cat ml-baseline-results/trex/similarity_eval2_libz_O0O3.csv.formatted.csv|egrep "^inflateMark-" > CVE-cases/case-9842.trex.csv
cat ml-baseline-results/trex/similarity_eval2_openssl101f_O0O3.csv.formatted.csv|egrep "^X509_verify_cert-" > CVE-cases/case-4044.trex.csv
cat ml-baseline-results/trex/similarity_eval2_openssl101f_O0O3.csv.formatted.csv|egrep "^EVP_PKEY_decrypt-" > CVE-cases/case-3711.trex.csv

cat ml-baseline-results/safe/coreutils.gcc94.cmp03.safe.csv|egrep "^single_binary_main_chown," > CVE-cases/case-18018.safe.csv
cat ml-baseline-results/safe/find47.find.cmp03.safe.csv|egrep "^visit_old_format," > CVE-cases/case-2452.safe.csv
cat ml-baseline-results/safe/libmagick7.so.cmp03.safe.csv|egrep "^sRGBTransformImage," > CVE-cases/case-20311.safe.csv
cat ml-baseline-results/safe/libmagick7.so.cmp03.safe.csv|egrep "^WaveImage," > CVE-cases/case-20309.safe.csv
cat ml-baseline-results/safe/libmagick7.so.cmp03.safe.csv|egrep "^ComplexImages," > CVE-cases/case-13308.safe.csv
cat ml-baseline-results/safe/libz.so.cmp03.safe.csv|egrep "^inflateMark," > CVE-cases/case-9842.safe.csv
cat ml-baseline-results/safe/openssl101f.cmp03.safe.csv|egrep "^X509_verify_cert," > CVE-cases/case-4044.safe.csv
cat ml-baseline-results/safe/openssl101f.cmp03.safe.csv|egrep "^EVP_PKEY_decrypt," > CVE-cases/case-3711.safe.csv