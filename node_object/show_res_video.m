function show_res_video(topboxes, I, models)

topK = 4;
topboxes = nms(topboxes, 0.5);
topboxes = topboxes(1:min(topK,size(topboxes,1)),:);

%use colors where 'hot' aka red means high score, and 'cold' aka
%blue means low score
colors = jet(size(topboxes,1));
colors = colors(end:-1:1,:);
%subplot_tight(3,3, [1 2 3 4 5 6]);
clf;
subplot_tight(2,4, [1 2 3 4]);
imshow(I);

if size(topboxes,1)<1
    return;
end

padd = 20;
for q = size(topboxes,1):-1:1
  myPlotBox(topboxes(q,:),colors(q,:),3);
end

subplot_tight(2,4,5);
indx = topboxes(1,6);
imshow(pad_image(models{indx}.exI, padd, colors(1,:)));

if size(topboxes,1)>1
    subplot_tight(2,4,6);
    indx = topboxes(2,6);
    imshow(pad_image(models{indx}.exI, padd, colors(2,:)));
    im = imresize(models{indx}.exI, [topboxes(2,4) - topboxes(2,2), topboxes(2,3) - topboxes(2,1)]);
end
if size(topboxes,1)>2
    subplot_tight(2,4,7);
    indx = topboxes(3,6);
    imshow(pad_image(models{indx}.exI, padd, colors(3,:)));
    im = imresize(models{indx}.exI, [topboxes(2,4) - topboxes(2,2), topboxes(2,3) - topboxes(2,1)]);
end
if size(topboxes,1)>3
    subplot_tight(2,4,8);
    indx = topboxes(4,6);
    imshow(pad_image(models{indx}.exI, padd, colors(3,:)));
    im = imresize(models{indx}.exI, [topboxes(2,4) - topboxes(2,2), topboxes(2,3) - topboxes(2,1)]);
end
drawnow;