imagepath = ['image/image0000' '001' '.bmp'];

    % show image
    img = imread (imagepath);
    imshow (img);
    rect = getrect
    ite=1;
            xmin(ite) = rect(1);
            xmax(ite) = rect(1)+rect(3);
            ymin(ite) = rect(2);
            ymax(ite) = rect(2)+rect(4);
            if (xmin(ite)<0)
                xmin(ite) = 0;
            end
            if (ymin(ite)<0)
                ymin(ite) = 0;
            end
            if (xmax(ite)>size(img, 2))
                xmax(ite) = size(img, 2);
            end
            if (ymax(ite)>size(img, 1))
                ymax(ite) = size(img, 1);
            end
            
            xmin
            xmax
            ymin
            ymax