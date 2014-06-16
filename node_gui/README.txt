How to add objects to the gui:
    1. add a jpg image of the object to the node_gui/images/objects directory, make sure the name of the jpg image is equal to what the bayes net would report.  For example if the bayes net reported that an object "dentalfloss" was found, save a dentalfloss.jpg file into this directory
    2. add a jpg image of the object mask to the node_gui/imaeg/objects directory.  The name of the mask file should be objectname_mask.jpg.  For example if the object reporeted by the bayes net was "dentalfloss" you would save a dentalfloss_mask.jpg file to this directory.  If you do not want to make a mask for the new images, just save an all white image with the naming conventions listed above.  This means the background will not be cut out of the image, instead the entire image will be shown.
    To recap: to add an object to the gui you need to save two files into the node_gui/images/objects directory with the naming conventions: 
    objectname.jpg
    objectname_mask.jpg

    3. Add images for the results of that object.  To do this add images to the node_gui/images/info_images/command directory.  Replace command with the command that shows this results.  For example, if you wanted to update the info result for an item called "dentalfloss" you would save the image that should be shown as node_gui/images/info_images/info/dentalfloss.jpg.  Similarly if you wanted to update the image that is shown for the price command for the dentalfloss object, you would save an image as node_gui/images/info_images/price/dentalfloss.jpg.  Currently, the gui overrides whatever command it recieved from the bayes net and displays whatever is in the node_gui/images/info_images/info directory, reguardless of the command that was issued.

In order to incoproate face recogition into the gui:

If you want the faces to populate the same as objects, just follow the above instructions to add objects to the database, but instead add a picture of the face with the corresponding identifier.
If you do not want to incorporate taking face data into the gui, this is the only thing that needs to be done to include the face recognition.  Also make sure to connect the face detection/recognition nodes in ROS to the picture published by the GUI.

If you want to add a "taking data" component, the only thing that needs to be implemented is publishFaceImage() method.  And you only need to implement this if you want the image the gui pubishes to be on a different topic that the regular publishImage method.  If not, you can just put a call to publishImage() in this method.  
The variable that controls the number of images that are published on this topic is facePicsToSend (set in gui.hpp)

