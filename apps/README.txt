HOW TO READ & UNDERSTAND THE UNIVERAL INTERIM FORMAT

The .tsv files used are split into two columns

EXAMPLE

0     462137     FRAME TIMESTAMP
3434  97a        OCID1 ADC1
3022  a10        OCID2 ADC2
2991  3cd        OCID3 ADC3
-1    -1         End of frame
1     462356
3434  939
3022  9a4
2991  2cb
-1    -1
E

The first element of the first row is the the frame number;
the seceond element is the timestamp.

The successive rows after give the offline channel ID as the first element;
then the corresponding adc value respectively as the seceond (in hex).

The end of a frame packet's list of respective channels is denoted with negative values
(this helps make it identifiable for the purposes of data extraction)

At the end of the requested data series is the letter 'E' to denote end
There are no other *capital* letters present in the .tsv file, so this also helps identify the end of a series as a unique character identifier

This is especially useful for combining multiple threads of data from separate requests