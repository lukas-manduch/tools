class HomepageController < ApplicationController
  skip_before_action :verify_login, only: [:log_in_get, :log_in_post]

  def index
  end

  def log_in_get
    @user = User
  end

  def log_in_post
    logger.info "Logging in!"
    login_params
    user = User.find_by email: login_params[:email]
    if user.present?
      login_user user.id
      redirect_to "/"
      return
    end
    flash[:notice] =  "Invalid email or password!"
    redirect_to login_path
  end

  private
  def login_params
    params.require(:user).permit(:email, :password)
  end
end
