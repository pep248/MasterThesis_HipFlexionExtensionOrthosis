function syncMessages = get_led_sync(wkv, messagesFilename)
%GET_LED_SYNC Extract infos corresponding to LED sync events.
%   Extracts the take, session, lap and WKV index for each LED sync event
%   that can be found in the messages file.

messages = extract_messages(wkv, messagesFilename, 0);

isSyncMessage = @(m) startsWith(string(m), "SYNC_LED: ");
syncIndices = find(cellfun(isSyncMessage, {messages.text}));

extractInfos = @(m) {sscanf(m, 'SYNC_LED: take %i, session %i, lap %i.')'};
infos = cell2mat(cellfun(extractInfos, {messages(syncIndices).text}'));

syncMessages = struct('take', num2cell(infos(:,1)'), ...
                      'session', num2cell(infos(:,2)'), ...
                      'lap', num2cell(infos(:,3)'), ...
                      'index', {messages(syncIndices).index}, ...
                      'time', {messages(syncIndices).time}); 

end
