
# Quy trình làm task từ lúc nhận đến khi Done

## Mục tiêu
Thống nhất cách làm việc cho mọi dev từ lúc nhận task/story đến lúc PR được merge và card chuyển sang `Done`.

---

## Flow tổng quát

```text
Nhận task
-> đọc story + docs liên quan
-> check card Trello
-> research nếu cần
-> sync ngắn với lead / reviewer nếu có điểm chưa rõ
-> tạo branch
-> chuyển card sang In Progress
-> implement
-> self-check
-> cập nhật docs nếu cần
-> commit + push
-> mở PR
-> gắn PR link vào card
-> chuyển card sang In Review
-> review + fix
-> merge
-> cập nhật card/checklist
-> Done
````

---

## Bước 1 - Nhận task

Khi được assign một task/story, dev phải xác nhận rõ:

* story id là gì
* mục tiêu của story là gì
* acceptance criteria là gì
* scope làm tới đâu, không làm gì
* card Trello tương ứng là card nào

Nếu chưa rõ scope thì không code ngay, phải hỏi lại.

### Trello cần làm

* mở đúng card Trello
* kiểm tra `Story ID`, `Epic`, `Sprint`, `Estimate`, `Owner`
* đọc title và description của card
* đọc checklist hiện có trong card
* nếu card còn thiếu owner / estimate / reviewer thì báo lead cập nhật trước khi bắt đầu

---

## Bước 2 - Đọc tài liệu liên quan

Trước khi code, cần đọc các tài liệu liên quan theo đúng loại task:

### Nếu là task theo story

Ưu tiên đọc:

* `docs/stories/...`
* `docs/planning/epics.md`
* `docs/planning/architecture.md`

### Nếu là task build/tooling/repo

Ưu tiên đọc:

* `README.md`
* `docs/guidelines/...`
* `docs/support/...` nếu có

### Nếu là task business rule

Ưu tiên đọc:

* requirement docs
* architecture
* decision log nếu có

### Trello cần làm

* đối chiếu acceptance criteria trong story với mô tả trong card
* nếu card mô tả quá ngắn hoặc thiếu AC quan trọng thì update card hoặc comment bổ sung
* nếu có file/docs quan trọng để đọc trước, có thể dán link vào card

---

## Bước 3 - Research nếu cần

Dev được phép research thêm để hiểu task tốt hơn.

Nên research khi:

* logic khó hiểu
* tool/build/config chưa quen
* có business rule dễ hiểu sai
* cần thêm ngữ cảnh để code hoặc review

### Rule khi research

* tài liệu ngoài chỉ là tài liệu tham khảo
* source of truth vẫn là docs của project
* nếu research hữu ích cho team, có thể thêm docs hỗ trợ vào `docs/support/`

### Trello cần làm

* chưa cần chuyển card cột chỉ vì đang research
* nếu research làm lộ ra blocker hoặc scope chưa rõ, comment ngay trong card để team cùng thấy
* nếu research sinh ra docs hỗ trợ hữu ích, có thể note trong card rằng PR sẽ kèm docs support

---

## Bước 4 - Sync ngắn trước khi code

Nếu task có điểm mơ hồ, dev nên sync ngắn với lead hoặc reviewer trước khi code.

Các điểm nên chốt sớm:

* interface giữa module
* business rule chưa rõ
* file nào được sửa
* file nào không nên đụng
* story này có cần cập nhật docs không

Không cần họp dài. Chỉ cần chốt hướng để tránh làm sai từ đầu.

### Trello cần làm

* nếu có quyết định quan trọng sau khi sync, ghi ngắn vào card comment
* nếu card scope phải đổi, update description/checklist thay vì chỉ nói miệng
* nếu task chưa thể làm tiếp do chờ quyết định, chuyển card sang `Blocked`

---

## Bước 5 - Tạo branch

### Rule

* 1 card = 1 branch = 1 PR
* không push thẳng `main`

### Format branch

```text
feat-story-<id>-<short-name>
fix-story-<id>-<short-name>
refactor-story-<id>-<short-name>
docs-story-<id>-<short-name>
chore-<short-name>
docs-<short-name>
```

Ví dụ:

```text
feat-story-1.1-project-scaffold
feat-story-1.5-login-session
docs-story-1.1-build-notes
```

### Trello cần làm

* khi branch đã tạo và bắt đầu code thật, chuyển card sang `In Progress`
* mỗi dev không nên có hơn 1 card chính ở `In Progress`
* nếu board có custom field phù hợp, có thể ghi branch name vào comment/card description

---

## Bước 6 - Implement

Trong lúc code:

* bám đúng scope story
* không tự mở rộng scope
* không kéo thêm feature story khác vào cùng branch nếu không thật sự cần
* giữ code đúng module boundaries
* nếu phát hiện blocker thì báo sớm

### Rule khi đang làm

* card bắt đầu code thật thì chuyển sang `In Progress`
* nếu bị kẹt và chưa làm tiếp được thì chuyển sang `Blocked`

### Trello cần làm

* tick checklist theo tiến độ thực tế
* cập nhật card trước hoặc trong daily scrum
* nếu phát hiện scope creep, tách card mới thay vì nhét thêm vào card hiện tại
* nếu card bị block hơn 1 ngày phải escalate

---

## Bước 7 - Commit

### Format commit

```text
feat(<module>): <hanh-dong-ngan>
fix(<module>): <hanh-dong-ngan>
refactor(<module>): <hanh-dong-ngan>
docs(<module>): <hanh-dong-ngan>
chore(<module>): <hanh-dong-ngan>
test(<module>): <hanh-dong-ngan>
```

Ví dụ:

```text
feat(makefile): add initial build targets
feat(main): add scaffold entry point
feat(auth): add login flow
docs(auth): add session flow notes
```

### Rule commit

* mỗi commit nên làm 1 việc chính
* không cần chia commit quá nhỏ nếu story đơn giản
* commit phải phản ánh thay đổi thật
* ưu tiên dễ review hơn là quá hình thức

### Trello cần làm

* không cần update card cho từng commit nhỏ
* chỉ cần bảo đảm card vẫn phản ánh trạng thái thật: đang làm, blocked, review

---

## Bước 8 - Self-check trước khi mở PR

Trước khi mở PR, dev phải tự kiểm tra:

### Bắt buộc

* code/build chạy được nếu story yêu cầu
* acceptance criteria đã được so lại
* scope vẫn đúng với story
* không có phần thừa không liên quan

### Nếu task có liên quan

* check role/permission behavior
* check persistence behavior
* check validation/error paths
* check manual flow chính

### Nếu project có tool hỗ trợ

Chạy các lệnh liên quan như:

* build
* format
* tidy
* manual test flow

### Trello cần làm

* tick checklist item tương ứng như `Verify build/manual run`
* nếu phát hiện chưa đạt AC thì chưa move card sang `In Review`

---

## Bước 9 - Cập nhật docs nếu cần

Phải cập nhật docs trong cùng PR nếu thay đổi ảnh hưởng đến:

* interface giữa các module
* struct, constant, file format
* business rule
* build / run / format / tidy flow
* branch / PR / workflow rules

### Dev cũng được khuyến khích thêm docs hỗ trợ khi:

* story khó hiểu
* build/config khó cho người mới
* reviewer cần thêm ngữ cảnh
* có flow nên ghi lại cho người sau

### Vị trí docs hỗ trợ

```text
docs/support/
```

### Trello cần làm

* nếu docs là phần quan trọng của story, mention trong card comment hoặc checklist
* nếu PR có docs support riêng, ghi rõ trên card để reviewer biết

---

## Bước 10 - Mở PR

Khi self-check xong:

* push branch
* mở PR
* điền đầy đủ PR template
* gắn đúng story id / Trello card
* ghi rõ phần đã verify
* nếu có cập nhật docs thì ghi rõ docs nào

### Rule

* mỗi PR map đúng 1 card Trello
* PR title nên chứa story id nếu PR gắn với story

### Trello cần làm

* dán `PR Link` vào custom field hoặc card attachment
* chuyển card sang `In Review`
* checklist phải có trạng thái đã mở PR
* nếu board dùng automation, bảo đảm card đã được add checklist item `PR link attached` khi sang `In Review`

---

## Bước 11 - Review và sửa review

Sau khi có review:

* sửa comment
* commit/push lại vào cùng branch
* trả lời các comment quan trọng
* nếu phát sinh thay đổi lớn hơn story, phải sync lại với lead

Không merge khi còn unresolved review comments quan trọng.

### Trello cần làm

* card vẫn ở `In Review` trong lúc sửa review
* không chuyển lại `In Progress` trừ khi team có rule riêng
* nếu review phát hiện blocker ngoài khả năng xử lý ngay, có thể chuyển `Blocked`

---

## Bước 12 - Khi nào được Done

Một task/story chỉ được xem là `Done` khi:

1. implementation hoàn tất
2. self-check / verification đã làm
3. PR đã mở và review xong
4. không còn issue review quan trọng chưa xử lý
5. PR đã được approve
6. PR đã merge vào `main`
7. Trello card đã được cập nhật đúng trạng thái

### Trello cần làm

* chỉ chuyển card sang `Done` sau khi PR đã merge và verify xong
* `Done` không có nghĩa là “code local xong”
* bảo đảm `PR Link` vẫn gắn với card
* nếu card từng bị `blocked`, label/blocker liên quan phải được dọn theo rule board

---

## Rule Trello chốt cho team

* Mỗi dev cập nhật card trước hoặc trong daily scrum
* Mỗi dev chỉ có 1 card chính ở `In Progress`
* Card có PR thì ở `In Review`
* Card không làm tiếp được thì phải sang `Blocked`
* Card chỉ sang `Done` sau merge + verify
* Mỗi PR phải gắn đúng 1 card Trello
* Nếu scope nở ra, tách card mới thay vì nhét thêm vào card cũ

## Rule chốt dễ nhớ

* Nhận task -> đọc docs -> check card -> research nếu cần -> sync -> branch -> `In Progress` -> code -> self-check -> docs nếu cần -> PR -> gắn PR link -> `In Review` -> review -> merge -> `Done`
* 1 card = 1 branch = 1 PR
* Không push thẳng `main`
* Code đổi đáng kể thì docs phải đi kèm
* Chỉ `Done` sau khi PR đã được review và merge
