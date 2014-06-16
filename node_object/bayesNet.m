% Implement the following Bayesian network:
% C, CMso, CMvo depends on O and CMsc depends on C 
%
% Inputs:
% *Note: All matrices are assumed to be normalized and smoothed.
% O = Column vector of obj
% C = Matrix where row is object and col is command
% CMsc = Speech confusion matrix where row is actual command and col is 
%        detected command
% CMso = Speech confusion matrix where row is actual obj and col is 
%        detected obj
% CMvo = Vision confusion matrix where row is actual obj and col is 
%        detected obj
% sc = Detected speech command
% so = Detected speech obj
% vo = Detected vision obj
%
%Outputs:
% c = Predicted command
% o = Predicted obj
% p = Think that it's c and o with probability p 
function [c, o, p] = bayesNet(O, C, CMsc, CMso, CMvo, sc, so, vo)

% Row corresponds to obj and col corresponds to command. Each entry is
% P(c|o)*P(sc|c)*P(so|o)*P(vo|o)*P(o)
P = bsxfun(@times, bsxfun(@times, C, CMsc(:,sc)'), (CMso(:,so) .* CMvo(:, vo) .* O));

denom = sum(sum(P));
P = P./denom;

[p i] = max(P(:));
[o c] = ind2sub(size(P),i);
