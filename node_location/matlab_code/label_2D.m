close all;
clear all;
clc;

objNumber = 8;
startFrame = 301;
endFrame = 556;
imgNumber = 50;
objString = {'book' 'cap' 'cup' 'disk' 'sweater' 'toy' 'tumbler' 'umbrella'};

for i = startFrame:endFrame
    if (i<10)
        iii = ['00' num2str(i)];
    elseif (i<100)
        iii = ['0' num2str(i)];
    else
        iii = num2str(i);
    end
    imagepath = ['image/image0000' iii '.bmp'];
    labelpath = ['label/image0000' iii '.label'];
    label = dlmread (labelpath);
    [row col] =size(label);
    % show image
    img = imread (imagepath);
    imshow (img);
    for ite = 1:objNumber
        %s = ['draw bounding box for ' objString{ite} ' Y/N [Y]: '];
        %reply = input(s, 's');
        %if isempty(reply)
        %    xmin(ite) = 0;
        %    xmax(ite) = 0;
        %    ymin(ite) = 0;
        %    ymax(ite) = 0;
        %    flag(ite) = 0;        
        %elseif (reply == 'l' || reply=='L')
        s = ['draw bounding box for ' objString{ite}];
        disp(s);
            rect = getrect;
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
            flag(ite) = 1;
        %else 
        %    xmin(ite) = 0;
        %    xmax(ite) = 0;
        %    ymin(ite) = 0;
        %    ymax(ite) = 0;
        %    flag(ite) = 0;
        %end
    end
    for point=1:1:row 
        pointy = label(point, 1);
        pointx = label(point, 2);
        for ite=1:objNumber
            if (flag(ite) == 1)
                if (pointy<=ymax(ite) && pointy>=ymin(ite) && ...
                        pointx<=xmax(ite) && pointx>=xmin(ite))
                    label(point, ite+2) = label(point, ite+2) +1;
                end
            end
        end
    end
    % output
    %labelll = fopen(labelpath, 'w');
    %for point=1:1:row 
    %    fprintf (labelll, [num2str(xmin(ite)) ' ' num2str(xmax(ite)) ' ' ...
    %    num2str(ymin(ite)) ' ' num2str(ymax(ite)) ... '\n']);
    %end
    
    dlmwrite (labelpath, label);
    
    % save the file for bounding box
    boundpath = ['bound/image0000' iii '.bound'];
    bound = fopen(boundpath, 'w');
    fprintf (bound, 'syntax: obj xmin xmax ymin ymax\n');
    for ite = 1:objNumber
        fprintf (bound, [objString{ite} ' ' num2str(xmin(ite)) ' ' ...
            num2str(xmax(ite)) ' ' num2str(ymin(ite)) ' ' num2str(ymax(ite)) ...
            '\n']);
    end
    
end