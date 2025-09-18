package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
import lombok.Getter;
import lombok.Setter;

/**
 * 更新使用者名稱請求體
 *
 * @create 2025-09-18
 **/
@Getter
@Setter
public class UpdateUserNameRequest {
    @JsonProperty("user_name")
    @NotBlank(message = "138")
    @Size(max = 16, message = "139")
    private String userName;
}