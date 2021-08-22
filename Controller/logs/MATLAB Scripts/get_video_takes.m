function takes = get_video_takes(wkv, messagesFilename)
%GET_VIDEO_TAKES Extract video takes infos from messages logfile.

messages = extract_messages(wkv, messagesFilename, 0);

isStartStopMessage = @(m) startsWith(string(m), "Client message: Take");
videoIndices = cellfun(isStartStopMessage, {messages.text});
videoMessages = messages(videoIndices);

regPattern = 'Client message: Take (?<take>\d+)\s(?<type>.*?)\.';
extractInfos = @(m) {regexp(m, regPattern, 'names')};
infos = cell2mat(cellfun(extractInfos, {videoMessages.text}'));

takes = struct('take', cell(1,length(infos)), ...
               'start', [], 'stop', [], 'duration', []);

nextNonEmpty = 1;
for i=1:length(infos)
    existingTakeIndex = find(infos(i).take == [takes.take], 1);
    if isempty(existingTakeIndex)
        takes(nextNonEmpty).take = infos(i).take;
        existingTakeIndex = nextNonEmpty;
        nextNonEmpty = nextNonEmpty + 1;
    end
    
    if strcmp(infos(i).type, 'started')
        takes(existingTakeIndex).start = videoMessages(i).time;
    else
        takes(existingTakeIndex).stop = videoMessages(i).time;
    end
    
    if ~isempty(takes(existingTakeIndex).start) && ~isempty(takes(existingTakeIndex).stop)
        takes(existingTakeIndex).duration = takes(existingTakeIndex).stop - takes(existingTakeIndex).start;
    end
end

% Remove empty elements.
takes(cellfun(@isempty, {takes.take})) = [];

end