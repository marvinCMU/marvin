function show_res(topboxes,I, models, topK)

topboxes = nms(topboxes, 0.5);
topboxes = topboxes(1:min(topK,size(topboxes,1)),:);

%use colors where 'hot' aka red means high score, and 'cold' aka
%blue means low score
colors = jet(size(topboxes,1));
colors = colors(end:-1:1,:);
figure(1)
set(gcf,'color','white');
clf
% subplot_tight(3,3, [1 2 3 4 5 6]);
subplot_tight(3,7, [1:3 8:10 15:17]);
imshow(I);
axis image
axis off

if size(topboxes,1)<1
    return;
end

padd = 20;
for q = size(topboxes,1):-1:1
  myPlotBox(topboxes(q,:),colors(q,:),3);
end

% subplot_tight(3,3,7);
subplot_tight(3,7,4);
indx = topboxes(1,6);
rect = [models{indx}.gt_box(1:2) ...
    [models{indx}.gt_box(3:4) - models{indx}.gt_box(1:2)]];
im = imcrop(convert_to_I(models{indx}.I), rect);
imshow(pad_image(im, padd, colors(1,:)));

try
    im = imresize(im, [topboxes(1,4) - topboxes(1,2), topboxes(1,3) - topboxes(1,1)]);
    I(topboxes(1,2):topboxes(1,4),topboxes(1,1):topboxes(1,3),:) = im;
catch
    return;
end

if size(topboxes,1)>1
%     subplot_tight(3,3,8);
    subplot_tight(3,7,11);
    indx = topboxes(2,6);
    rect = [models{indx}.gt_box(1:2) ...
        [models{indx}.gt_box(3:4) - models{indx}.gt_box(1:2)]];
    im = imcrop(convert_to_I(models{indx}.I), rect);
    imshow(pad_image(im, padd, colors(2,:)));
im = imresize(im, [topboxes(2,4) - topboxes(2,2), topboxes(2,3) - topboxes(2,1)]);
try
I(topboxes(2,2):topboxes(2,4),topboxes(2,1):topboxes(2,3),:) = im;
catch
    return;
end
end
if size(topboxes,1)>2
%     subplot_tight(3,3,9);
    subplot_tight(3,7,18);
    indx = topboxes(3,6);
    rect = [models{indx}.gt_box(1:2) ...
        [models{indx}.gt_box(3:4) - models{indx}.gt_box(1:2)]];
    im = imcrop(convert_to_I(models{indx}.I), rect);
    imshow(pad_image(im, padd, colors(3,:)));
    im = imresize(im, [topboxes(2,4) - topboxes(2,2), topboxes(2,3) - topboxes(2,1)]);
    try
I(topboxes(2,2):topboxes(2,4),topboxes(2,1):topboxes(2,3),:) = im;
catch
    return;
end
end
% subplot_tight(3,7, [5:7 12:14 19:21]);
% imshow(I);
drawnow;