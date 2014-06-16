>labels.csv
cp default_model.xml model.xml
> model.xml.index2Label.txt
ls -al | grep ^d | grep -v image_labels | xargs rm -r
rm ./image_labels/*
