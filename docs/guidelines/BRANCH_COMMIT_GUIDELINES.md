
# Quy định đặt tên branch và commit message

## Mục tiêu
Thống nhất cách đặt tên branch và commit để:
- nhìn vào là biết đang làm story nào
- review và trace lịch sử dễ hơn
- mỗi card Trello khớp với một branch và một PR
- code và docs đều có quy trình rõ ràng

## Nguyên tắc chung
- 1 card = 1 branch = 1 PR
- tên branch phải chứa `story-id` nếu là work theo story
- dùng chữ thường
- dùng dấu `-`, không dùng khoảng trắng
- tên ngắn, rõ nghĩa, không lan man
- merge xong thì xóa branch
- không push thẳng `main`
- chỉ merge sau khi review xong

## Quy tắc đặt tên branch

### Format chuẩn cho story
```text
<type>-story-<story-id>-<mo-ta-ngan>
````

### Các `type` dùng trong team

* `feat`: tính năng mới
* `fix`: sửa lỗi
* `chore`: setup, config, tooling, dọn repo
* `docs`: tài liệu
* `refactor`: chỉnh cấu trúc code nhưng không đổi hành vi

### Ví dụ branch đúng

```text
feat-story-1.1-project-scaffold
feat-story-1.2-shared-types
feat-story-1.3-utility-helpers
feat-story-1.4-fileio-init
feat-story-1.5-login-session
feat-story-2.1-add-member
fix-story-1.5-login-lockout
refactor-story-2.2-member-edit-flow
chore-setup-clang-format
docs-branch-commit-guidelines
```

### Quy ước rút gọn mô tả branch

* dùng 2 đến 5 từ là đủ
* ưu tiên động từ + đối tượng
* tránh tên quá dài

Ví dụ tốt:

* `project-scaffold`
* `shared-types`
* `login-session`
* `add-member`
* `password-reset`

Ví dụ không tốt:

* `do-something-for-auth-module-and-main-menu`
* `branch-moi`
* `code-tam`

## Quy tắc commit message

### Format chuẩn

```text
<type>(<module>): <hanh-dong-ngan>
```

### Các `type` nên dùng

* `feat`: thêm chức năng
* `fix`: sửa bug
* `refactor`: sửa cấu trúc code
* `docs`: sửa/thêm tài liệu
* `chore`: việc phụ trợ như config, format, lint
* `test`: thêm/sửa test nếu có

### Module nên dùng theo dự án

* `main`
* `auth`
* `member`
* `violation`
* `fileio`
* `report`
* `utils`
* `types`
* `makefile`
* `repo`
* `readme`
* `architecture`
* `docs`

### Ví dụ commit đúng

```text
feat(makefile): add initial build targets
feat(main): add scaffold entry point
feat(types): define shared structs and constants
feat(utils): add email validation helper
feat(fileio): add first-run data initialization
feat(auth): add login and session flow
fix(auth): reset failCount after successful login
fix(fileio): save accounts after password reset
refactor(main): simplify menu routing
docs(readme): add build instructions
chore(repo): add clang-format config
```

### Ví dụ commit không nên dùng

```text
update code
fix bug
done story 1.2
aaa
sua code
```

## Quy tắc commit tốt

Một commit tốt nên:

* chỉ làm 1 việc chính
* đọc message là hiểu sửa gì
* không trộn nhiều thay đổi không liên quan
* đủ nhỏ để review nhanh

### Lưu ý quan trọng

* không bắt buộc phải chia quá nhiều commit nếu story nhỏ
* 1 story có thể chỉ cần 1 đến 3 commit rõ nghĩa
* commit nên phản ánh thay đổi thật, không tách commit cho “đẹp lịch sử” nếu không cần
* ưu tiên dễ review hơn là quá hình thức

## Áp dụng cho Sprint 1

### Branch gợi ý

```text
feat-story-1.1-project-scaffold
feat-story-1.2-shared-types
feat-story-1.3-utility-helpers
feat-story-1.4-fileio-init
feat-story-1.5-login-session
feat-story-1.6-password-reset
feat-story-2.1-add-member
```

### Commit gợi ý theo story

#### Story 1.1

```text
feat(makefile): add initial build targets
feat(main): add scaffold entry point
```

Hoặc nếu thay đổi nhỏ và làm trong một lần:

```text
feat(makefile): add project scaffold and build setup
```

#### Story 1.2

```text
feat(types): define shared structs and constants
```

#### Story 1.3

```text
feat(utils): add validation and helper functions
```

#### Story 1.4

```text
feat(fileio): add load save and first-run initialization
```

#### Story 1.5

```text
feat(auth): add login logout and session flow
fix(auth): lock account after three failed attempts
```

## Quy tắc khi sửa technical docs / project docs

### Khi nào được xem là sửa docs

Bao gồm các file như:

* `README.md`
* `architecture.md`
* `epics.md`
* `trello-board-blueprint.md`
* `BRANCH_COMMIT_GUIDELINES.md`
* `decision-log.md`
* `agreed-interfaces.md`
* các file trong thư mục `docs/`

### Khi nào bắt buộc phải cập nhật docs

Nếu PR làm thay đổi một trong các nội dung sau thì phải cập nhật docs liên quan trong cùng PR hoặc ghi rõ vì sao chưa cần:

* interface giữa các module
* struct, constant, file format
* business rule
* build / run / format / tidy flow
* naming convention, branch rule, PR rule
* cách team vận hành board / sprint / review

### Branch khi sửa docs

Nếu chỉ sửa docs:

```text
docs-<short-name>
```

Ví dụ:

```text
docs-readme-build-guide
docs-architecture-update
docs-branch-commit-guidelines
docs-decision-log-out-clb
```

Nếu docs gắn trực tiếp với một story đang làm:

```text
docs-story-<story-id>-<short-name>
```

Ví dụ:

```text
docs-story-1.1-build-guide
docs-story-1.4-fileio-notes
docs-story-1.5-auth-flow-notes
```

### Commit khi sửa docs

Format:

```text
docs(<module>): <hanh-dong-ngan>
```

Ví dụ:

```text
docs(readme): add build and run instructions
docs(architecture): clarify fileio save flow
docs(auth): add login flow notes
docs(repo): update branch and commit guidelines
docs(decision-log): record out clb threshold decision
```

## Quy tắc khi dev thêm docs để team hiểu phần mình làm

### Mục đích

Dev được khuyến khích thêm docs ngắn để:

* giải thích phần mình đang làm
* giúp reviewer và người khác đọc code nhanh hơn
* giảm phụ thuộc vào chat miệng

### Loại docs nên thêm

Ví dụ:

* `docs/auth-flow.md`
* `docs/fileio-notes.md`
* `docs/member-module-guide.md`
* `docs/how-build-works.md`
* `docs/story-1.1-notes.md`

### Khi nào nên thêm docs hỗ trợ

Nên thêm khi:

* story có logic khó hiểu
* story có build/tooling/config khó cho người mới
* có business rule dễ hiểu sai
* có quyết định kỹ thuật cần người sau đọc lại
* reviewer cần ngữ cảnh để review nhanh hơn

### Vị trí docs hỗ trợ
Tất cả tài liệu hỗ trợ để giúp team hiểu nhanh hơn về story, module, flow, build, hoặc technical notes nên đặt trong:
`docs/support/`

Các file trong `docs/support/` là tài liệu hỗ trợ đọc hiểu, không thay thế source of truth của project như:
- `docs/requirement-docs/`
- `architecture.md`
- `epics.md`
- `decision-log.md`
- các guideline chính trong `docs/`

### Rule cho docs hỗ trợ

* docs phải ngắn, rõ, có ích
* không copy nguyên tài liệu dài từ blog ngoài
* không thay thế source of truth của project
* nếu có link tài liệu ngoài thì chỉ để tham khảo
* nếu docs gắn với story nào thì nên ghi rõ story id

### Branch và commit cho docs hỗ trợ

Nếu viết docs hỗ trợ riêng:

```text
docs-story-<story-id>-<short-name>
```

Ví dụ:

```text
docs-story-1.1-makefile-notes
docs-story-1.5-auth-session-notes
```

Commit ví dụ:

```text
docs(makefile): add story 1.1 build notes
docs(auth): add session flow explanation
docs(fileio): add first-run initialization notes
```

### PR rule cho docs

* nếu docs đi cùng code của story thì để chung PR với story đó
* nếu chỉ bổ sung docs cho dễ hiểu hơn, có thể mở PR docs riêng
* PR phải ghi rõ:

  * docs này hỗ trợ story/module nào
  * docs này là source of truth hay chỉ là note hỗ trợ

## Rule chốt cho team

```text
Branch:
feat-story-<id>-<short-name>
fix-story-<id>-<short-name>
refactor-story-<id>-<short-name>
docs-story-<id>-<short-name>
chore-<short-name>
docs-<short-name>

Commit:
feat(<module>): ...
fix(<module>): ...
refactor(<module>): ...
docs(<module>): ...
chore(<module>): ...
test(<module>): ...
```

## PR rule đi kèm

* PR title nên chứa story id nếu PR gắn với story
* mỗi PR map đúng 1 card Trello
* không push thẳng `main`
* merge chỉ sau khi review xong
* nếu PR có thay đổi docs thì mô tả rõ docs nào đã cập nhật
* nếu PR không cập nhật docs dù có thay đổi cách hiểu hệ thống thì phải ghi rõ lý do

## Rule chốt về docs

* Code đổi gì làm ảnh hưởng cách hiểu hệ thống thì docs liên quan phải được cập nhật.
* Dev được khuyến khích thêm docs ngắn để team hiểu phần mình làm.
* Docs hỗ trợ để giải thích thêm, không thay thế `architecture`, `epics`, hoặc `decision-log`.

## Câu chốt dễ nhớ

* Branch = `type-story-id-mo-ta` hoặc `docs-mo-ta`
* Commit = `type(module): hanh-dong`
* Không cần chia commit quá nhỏ nếu story đơn giản
* Code đổi đáng kể thì docs phải đi kèm
