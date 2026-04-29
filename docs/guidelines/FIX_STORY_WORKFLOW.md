# Quy trình sửa story đã Done

## Mục tiêu
Thống nhất cách xử lý khi một story đã ở trạng thái `Done` nhưng phát sinh nhu cầu sửa tiếp vì bug, reviewer feedback muộn, requirement change, docs mismatch, hoặc config/tooling chưa đúng.

---

## Nguyên tắc chốt

- không âm thầm làm tiếp trên card cũ đã `Done`
- thay đổi đáng kể phải đi qua `card mới + branch mới + PR mới`
- vẫn giữ rule `1 card = 1 branch = 1 PR`
- card/story cũ giữ nguyên lịch sử, không dùng lại để chứa scope mới
- card mới phải link ngược về story cũ để trace được nguồn gốc

---

## Khi nào phải tạo card mới

Phải tạo card mới nếu thuộc một trong các trường hợp sau:

- sửa bug hoặc behavior của story đã `Done`
- bổ sung scope sau khi story đã merge
- sửa theo feedback đến muộn sau khi card đã `Done`
- chỉnh docs/config/tooling vì đang mâu thuẫn với requirement hoặc implementation
- thay đổi đủ lớn để vẫn cần review, branch, và PR riêng

Ví dụ:

- story 1.2 đã done nhưng cần sửa naming theo requirement
- story 1.1 đã done nhưng `.clang-tidy` đang enforce naming sai
- story đã merge nhưng review sau đó phát hiện regression

---

## Khi nào không cần tạo card mới

Có thể sửa trực tiếp rất nhẹ nếu chỉ là:

- typo nhỏ
- wording nhỏ không đổi meaning
- fix markdown/link hỏng nhưng không đổi scope hay rule

Nếu còn phân vân, ưu tiên tạo card mới.

---

## Flow chuẩn

```text
phát hiện need fix
-> đọc lại story cũ + docs liên quan
-> xác định loại fix
-> tạo card Trello mới
-> link về story/card cũ
-> tạo branch mới từ main
-> chuyển card mới sang In Progress
-> implement
-> self-check
-> update docs nếu cần
-> commit + push
-> mở PR mới
-> gắn PR link vào card mới
-> chuyển card mới sang In Review
-> review + fix review
-> merge
-> verify
-> Done
```

---

## Bước 1 - Đọc lại ngữ cảnh

Trước khi sửa, dev phải đọc lại:

- story cũ
- acceptance criteria cũ
- requirement docs nếu có liên quan
- architecture / epics / guideline liên quan
- PR cũ hoặc review comment cũ nếu cần

Mục tiêu là xác định rõ:

- đây là bug, follow-up, hay scope change
- fix này có thay đổi behavior không
- fix này có cần update docs/config không

---

## Bước 2 - Tạo card Trello mới

Không tái sử dụng card cũ đã `Done` cho một thay đổi đáng kể.

### Tên card gợi ý

```text
Fix Story <story-id> - <mo-ta-ngan>
Follow-up Story <story-id> - <mo-ta-ngan>
Docs follow-up Story <story-id> - <mo-ta-ngan>
Chore follow-up Story <story-id> - <mo-ta-ngan>
```

Ví dụ:

```text
Fix Story 1.2 - align naming with requirement
Docs follow-up Story 1.2 - clarify field naming
Chore follow-up Story 1.1 - update clang-tidy naming
```

### Card mới cần có

- story id gốc bị ảnh hưởng
- link card/story cũ
- lý do mở fix
- scope sẽ sửa
- out of scope
- owner
- estimate nếu board đang dùng estimate

### Comment/link nên ghi trong card mới

```text
Related to Story 1.2.
Reason: requirement-first naming alignment after story was already Done.
Original card: <link-card-cu>
```

### Trello cần làm

- tạo card mới ở đúng cột backlog/sprint
- không kéo card cũ từ `Done` về `In Progress` chỉ để làm fix mới
- nếu fix cần làm ngay trong sprint hiện tại, move card mới vào `Sprint Backlog`

---

## Bước 3 - Tạo branch mới

Branch phải tách riêng khỏi branch cũ đã merge.

### Rule

- branch mới luôn tạo từ `main`
- không revive branch cũ đã merge
- không gộp nhiều fix không liên quan vào cùng branch

### Format branch

Nếu là sửa code:

```text
fix-story-<story-id>-<short-name>
```

Nếu là docs:

```text
docs-story-<story-id>-<short-name>
```

Nếu là config/tooling:

```text
chore-story-<story-id>-<short-name>
```

Ví dụ:

```text
fix-story-1.2-naming-alignment
docs-story-1.2-requirement-naming
chore-story-1.1-clang-tidy-naming
```

### Trello cần làm

- khi branch đã tạo và bắt đầu làm thật, chuyển card mới sang `In Progress`
- nếu có custom field phù hợp, ghi branch name vào card

---

## Bước 4 - Implement đúng scope fix

Trong lúc sửa:

- chỉ sửa đúng vấn đề của card mới
- không tiện tay nhét thêm scope khác
- nếu phát hiện thêm vấn đề khác đáng kể, tách card khác

### Ví dụ scope hợp lệ

- sửa logic xử lý
- chỉnh lại docs theo requirement
- sửa lint/tooling đang enforce rule sai

### Ví dụ scope không nên nhét thêm

- đang fix naming nhưng tiện tay refactor module lớn
- đang sửa docs nhưng kéo thêm feature mới

---

## Bước 5 - Self-check

Trước khi mở PR, cần tự kiểm:

- fix có đúng mục tiêu card mới không
- có làm lệch acceptance criteria cũ không
- có phát sinh update docs/config không
- build / format / tidy / manual flow liên quan có ổn không

Nếu fix chạm vào rule/team convention thì phải update docs trong cùng PR hoặc ghi rõ lý do chưa update.

---

## Bước 6 - Commit

### Format commit

```text
fix(<module>): <hanh-dong-ngan>
docs(<module>): <hanh-dong-ngan>
chore(<module>): <hanh-dong-ngan>
```

Ví dụ:

```text
fix(types): align field naming with requirement
docs(architecture): clarify requirement-first naming
chore(repo): update clang-tidy naming rules
```

---

## Bước 7 - Mở PR mới

Fix cho story đã `Done` vẫn phải có PR riêng.

### PR cần ghi rõ

- card Trello mới nào
- story cũ nào bị ảnh hưởng
- vì sao cần follow-up sau khi đã `Done`
- đã verify gì

### Title gợi ý

```text
fix(story 1.2): align naming with requirement
docs(story 1.2): clarify field naming convention
chore(story 1.1): update clang-tidy naming config
```

### Trello cần làm

- dán `PR Link` vào card mới
- chuyển card mới sang `In Review`
- card cũ vẫn giữ `Done`

---

## Bước 8 - Review, merge, Done

Sau review:

- sửa review trên cùng branch mới
- push lại vào cùng PR
- chỉ merge khi review xong

Card mới chỉ được sang `Done` khi:

1. fix đã hoàn tất
2. self-check đã xong
3. PR đã được review
4. PR đã merge
5. đã verify sau merge nếu cần

---

## Rule Trello chốt cho case fix story

- story cũ đã `Done` thì không dùng lại cho scope fix đáng kể
- luôn tạo card mới nếu fix cần branch và PR riêng
- card mới phải link về card/story cũ
- card mới mới là nơi gắn `PR Link`
- card cũ giữ nguyên `Done` để bảo toàn lịch sử

---

## Rule chốt dễ nhớ

- story đã `Done` mà cần sửa đáng kể: `tạo card mới, branch mới, PR mới`
- không reopen âm thầm card cũ để nhét thêm scope
- trace phải rõ: card mới luôn link về story cũ
