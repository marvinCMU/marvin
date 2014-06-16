dd if=general.txt of=general_lc.txt conv=lcase
./prepare_corpus.py general_lc.txt general_corpus.txt
#Just in case:
dd if=general_corpus.txt of=general_corpus_lc.txt conv=lcase

#text2wfreq < general_lc.txt | wfreq2vocab > general.tmp.vocab
