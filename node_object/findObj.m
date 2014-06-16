% Obj recognition on given frame based on loaded models
function detected_model = findObj(frame, timestamp)

detection_images = '~/data/vision_processed/';
detected_models = '~/data/model_processed/'; 

I = convert_to_I(imread(frame));
models = evalin('caller', 'models');
%find best detections
boxes = my_find_exemp(I, models);

topK = 4;
topboxes = nms(boxes, 0.5);
topboxes = topboxes(1:min(topK,size(topboxes,1)),:);

%use colors where 'hot' aka red means high score, and 'cold' aka
%blue means low score
colors = jet(size(topboxes,1));
colors = colors(end:-1:1,:);

set(gcf, 'visible', 'off');
clf;
imshow(I, 'Border', 'tight');

% nothing is detected 
if size(topboxes,1)<1
    detected_model = 'none';
    blank_im = ones(15, 20);
    imshow(blank_im, 'Border', 'tight');
    saveas(gcf, [detection_images 't_' timestamp{1} '.jpg']);
    pause(0.01);
    saveas(gcf, [detected_models 'm_' timestamp{1} '.jpg']);
    pause(0.01);
    return;
end

for q = size(topboxes,1):-1:1
  myPlotBox(topboxes(q,:),colors(q,:),3);
end

saveas(gcf, [detection_images 't_' timestamp{1} '.jpg']);
pause(0.01);

clf;
padd = 20;
indx = topboxes(1,6);
imshow(pad_image(models{indx}.exI, padd, colors(1,:)));
saveas(gcf, [detected_models 'm_' timestamp{1} '.jpg']);
pause(0.01);
detected_model = models{indx}.cls;